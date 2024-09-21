#include <windows.h>               // Inclui a API do Windows
#include <setupapi.h>             // Inclui a API para manipulação de dispositivos
#include <devguid.h>              // Inclui identificadores de classes de dispositivos
#include <iostream>               // Inclui a biblioteca de entrada e saída padrão
#include <string>                 // Inclui a biblioteca de strings
#include <vector>                 // Inclui a biblioteca de vetores
#include <locale>                 // Inclui a biblioteca para manipulação de localidade
#include <stdexcept>              // Inclui a biblioteca para exceções padrão

#pragma comment(lib, "setupapi.lib") // Linka com a biblioteca SetupAPI

// Estrutura para armazenar informações de dispositivos
struct DeviceInfo {
    std::wstring deviceId;      // ID do dispositivo
    std::wstring deviceName;    // Nome do dispositivo
};

// Função para listar controladores de som, vídeo e jogos
std::vector<DeviceInfo> ListAudioVideoGameControllers() {
    std::vector<DeviceInfo> devices; // Vetor para armazenar os dispositivos encontrados

    // Obtém a lista de dispositivos da classe MEDIA (som, vídeo e jogos)
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_MEDIA, NULL, NULL, DIGCF_PRESENT);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) { // Verifica se houve erro ao obter a lista
        std::wcerr << L"Erro ao obter dispositivos: " << GetLastError() << std::endl;
        return devices; // Retorna lista vazia em caso de erro
    }

    SP_DEVINFO_DATA deviceInfoData; // Estrutura para armazenar informações do dispositivo
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA); // Define o tamanho da estrutura

    // Itera sobre os dispositivos encontrados
    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); ++i) {
        WCHAR nameBuffer[256]; // Buffer para armazenar o nome do dispositivo
        if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData,
            SPDRP_DEVICEDESC, NULL, (PBYTE)nameBuffer, sizeof(nameBuffer), NULL)) {

            // Obtém o ID do dispositivo
            WCHAR idBuffer[256]; // Buffer para armazenar o ID do dispositivo
            if (SetupDiGetDeviceInstanceId(deviceInfoSet, &deviceInfoData, idBuffer, sizeof(idBuffer) / sizeof(WCHAR), NULL)) {
                std::wstring deviceId = idBuffer; // Armazena o ID do dispositivo

                // Adiciona à lista de dispositivos
                devices.push_back({ deviceId, nameBuffer }); // Adiciona um novo dispositivo ao vetor
            }
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet); // Libera a lista de dispositivos
    return devices; // Retorna a lista de dispositivos encontrados
}

// Função para desinstalar um dispositivo dado seu ID
void UninstallDevice(const std::wstring& deviceId) {
    GUID interfaceClassGuid = GUID_DEVCLASS_MEDIA; // Define a classe de dispositivos a ser desinstalada
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&interfaceClassGuid, NULL, NULL, DIGCF_PRESENT); // Obtém a lista de dispositivos

    if (deviceInfoSet == INVALID_HANDLE_VALUE) { // Verifica se houve erro ao obter a lista
        std::wcerr << L"Erro ao obter a lista de dispositivos para desinstalação: " << GetLastError() << std::endl;
        return; // Retorna em caso de erro
    }

    SP_DEVINFO_DATA deviceInfoData; // Estrutura para armazenar informações do dispositivo
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA); // Define o tamanho da estrutura

    // Encontra o dispositivo pelo ID
    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); ++i) {
        WCHAR idBuffer[256]; // Buffer para armazenar o ID do dispositivo
        if (SetupDiGetDeviceInstanceId(deviceInfoSet, &deviceInfoData, idBuffer, sizeof(idBuffer) / sizeof(WCHAR), NULL)) {
            if (deviceId == idBuffer) { // Verifica se o ID do dispositivo corresponde ao ID fornecido
                // Remove o dispositivo
                if (SetupDiCallClassInstaller(DIF_REMOVE, deviceInfoSet, &deviceInfoData)) {
                    std::wcout << L"Dispositivo desinstalado com sucesso." << std::endl; // Mensagem de sucesso
                }
                else {
                    std::wcerr << L"Erro ao desinstalar o dispositivo: " << GetLastError() << std::endl; // Mensagem de erro
                }
                break; // Sai do loop após encontrar e tentar desinstalar o dispositivo
            }
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet); // Libera a lista de dispositivos
}

// Função para adicionar um comando de desinstalação ao Registro para execução na inicialização
void AddStartupUninstall(const std::wstring& deviceId) {
    HKEY hKey; // Handle para a chave do Registro
    if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) { // Cria ou abre a chave
        // Comando para desinstalar o dispositivo usando PowerShell
        std::wstring command = L"powershell.exe -Command \"Get-PnpDevice -InstanceId '" + deviceId + L"' | Disable-PnpDevice -Confirm:$false\"";
        // Adiciona o comando à chave do Registro
        RegSetValueEx(hKey, deviceId.c_str(), 0, REG_SZ, (const BYTE*)command.c_str(), (command.size() + 1) * sizeof(wchar_t));
        RegCloseKey(hKey); // Fecha a chave do Registro
        std::wcout << L"Comando adicionado ao registro para desinstalar o dispositivo na inicialização." << std::endl; // Mensagem de sucesso
    }
    else {
        std::wcerr << L"Erro ao acessar o Registro: " << GetLastError() << std::endl; // Mensagem de erro
    }
}

// Função principal
int main() {
    setlocale(LC_ALL, "pt_BR"); // Define a localidade para português do Brasil

    std::vector<DeviceInfo> devices = ListAudioVideoGameControllers(); // Lista os dispositivos
    if (devices.empty()) { // Verifica se nenhum dispositivo foi encontrado
        std::wcout << L"Nenhum controlador de som, vídeo ou jogos encontrado." << std::endl; // Mensagem informativa
        return 0; // Sai do programa
    }

    // Exibe os dispositivos encontrados
    std::wcout << L"Controladores de som, vídeo e jogos encontrados:\n";
    for (size_t i = 0; i < devices.size(); ++i) {
        std::wcout << i + 1 << L": " << devices[i].deviceName << L" (ID: " << devices[i].deviceId << L")" << std::endl; // Exibe cada dispositivo
    }

    std::wcout << L"\nSelecione um dispositivo para desinstalar (ou digite 0 para sair): "; // Solicita a entrada do usuário
    int choice; // Variável para armazenar a escolha do usuário
    std::wcin >> choice; // Lê a escolha do usuário

    // Processa a escolha do usuário
    if (choice > 0 && choice <= static_cast<int>(devices.size())) {
        const std::wstring& selectedDeviceId = devices[choice - 1].deviceId; // Obtém o ID do dispositivo selecionado
        UninstallDevice(selectedDeviceId); // Chama a função para desinstalar o dispositivo
        AddStartupUninstall(selectedDeviceId); // Adiciona o comando de desinstalação ao Registro
    }
    else if (choice == 0) {
        std::wcout << L"Saindo..." << std::endl; // Mensagem de saída
    }
    else {
        std::wcout << L"Escolha inválida." << std::endl; // Mensagem de erro para escolha inválida
    }

    system("pause"); // Pausa o sistema antes de sair
    return 0; // Retorna 0 ao sistema operacional
}
