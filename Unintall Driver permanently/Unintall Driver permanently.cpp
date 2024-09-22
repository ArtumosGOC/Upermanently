#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <clocale>
#include <stdexcept>
#include <sstream>
#include <conio.h>
#include <algorithm>


#pragma comment(lib, "setupapi.lib")

// Estrutura para armazenar informações sobre dispositivos
struct DeviceInfo {
    std::wstring deviceId;   // ID do dispositivo
    std::wstring deviceName; // Nome do dispositivo
	std::wstring status;     // Status do dispositivo
};

// Função para substituir "\\" por "\"
std::wstring splitByDoubleBackslash(const std::wstring& str) {
    std::wstring result;
    std::wstring delimiter = L"\\\\";  // O delimitador "\\"
    size_t pos = 0, prevPos = 0;

    // Substitui ocorrências de "\\" por "\"
    while ((pos = str.find(delimiter, prevPos)) != std::wstring::npos) {
        result += str.substr(prevPos, pos - prevPos) + L"\\";
        prevPos = pos + delimiter.length();
    }
    result += str.substr(prevPos);  // Adiciona a última parte
    return result;
}

// Função para substituir sequências de escape em strings
std::wstring ReplaceEscapeSequences(const std::wstring& str) {
    std::wstring result = str;

    // Definir pares de sequências a serem substituídas
    std::vector<std::pair<std::wstring, std::wstring>> replacements = {
        {L"\\u0026", L"&"}, // Substitui \u0026 por &
        {L"\\u0027", L"'"},
        {L"\\u0022", L"\""},
        {L"\\u005C", L"\\"}, // Substitui \\ por \
        // Adicione mais pares conforme necessário
    };

    // Substituir cada sequência
    for (const auto& pair : replacements) {
        size_t pos = 0;
        while ((pos = result.find(pair.first, pos)) != std::wstring::npos) {
            result.replace(pos, pair.first.length(), pair.second);
            pos += pair.second.length(); // Avança para evitar loop infinito
        }
    }

    // Formatar o deviceId conforme necessário (remover espaços extras)
    result.erase(remove(result.begin(), result.end(), L' '), result.end()); // Remove espaços em branco

    return result;
}
std::wstring TextUpper(const std::wstring& text)
{
	std::wstring result = text;
	std::transform(result.begin(), result.end(), result.begin(), ::toupper);
	return result;
}
// Função para executar um comando PowerShell e capturar a saída
std::wstring ExecutePowerShellCommand(const std::wstring& command) {
    std::wstring result;
    FILE* pipe = _wpopen(command.c_str(), L"r");
    if (!pipe) {
        throw std::runtime_error("Falha ao abrir pipe");
    }

    wchar_t buffer[128];
    while (fgetws(buffer, sizeof(buffer) / sizeof(wchar_t), pipe) != NULL) {
        result += buffer; // Adiciona a saída do buffer ao resultado
    }
    _pclose(pipe);
    return result;
}

// Função para listar dispositivos BTHENUM
std::vector<DeviceInfo> ListDevices() {
	
    std::vector<DeviceInfo> devices;
    std::wstring command = L"powershell.exe -Command \"Get-PnpDevice | Select-Object InstanceId, FriendlyName, Status | ConvertTo-Json\"";
    std::wstring output = ExecutePowerShellCommand(command);

    // Processar a saída JSON
    std::wistringstream jsonStream(output);
    std::wstring line;

    
	std::wcout << L"Qual opcao deseja realizar?\n1 - Desinstalar um driver\n2 - Listar drivers desinstalados\n";

	int option;

    while (true) {
        if (_kbhit()) { // Verifica se uma tecla foi pressionada
            char ch = _getch(); // Captura a tecla pressionada

            if (ch == '1') {
				option = 1;
                break; // Sai do loop
            }
            else if (ch == '2') {
				option = 2;
                break; // Sai do loop
            }
        }
    }
	system("cls"); // Limpa a tela

    if (option == 1)
    {
        // Solicitar palavra-chave do usuário para filtrar dispositivos
        std::wstring keyword;
		std::wstring isFriendlyName;

		std::wcout << L"Nome do dispositivo ou nome do Servico do dispositivo:( D - Dispositivo, S - Servico)\n";
        while (true) {
            if (_kbhit()) { // Verifica se uma tecla foi pressionada
                char ch = _getch(); // Captura a tecla pressionada

                if (ch == 's' || ch == 'S') {
					isFriendlyName = L"S";
                    break; // Sai do loop
                }
                else if (ch == 'd' || ch == 'D') {
					isFriendlyName = L"D";
                    break; // Sai do loop
                }
            }
        }

		system("cls"); // Limpa a tela

        std::wcout << L"Digite a palavra-chave para filtrar dispositivos (deixe vazio para listar ): ";
        std::getline(std::wcin, keyword);
        keyword.erase(std::remove(keyword.begin(), keyword.end(), L' '), keyword.end()); // Remover espaços em branco
        if (isFriendlyName == L"S")
		    keyword = TextUpper(keyword);

        while (std::getline(jsonStream, line)) {
            if (line.find(L"InstanceId") != std::wstring::npos) {
                // Extrair o InstanceId
                std::wstring instanceId = line.substr(line.find(L":") + 1);
                instanceId.erase(remove(instanceId.begin(), instanceId.end(), L'"'), instanceId.end());
                instanceId.erase(remove(instanceId.begin(), instanceId.end(), L','), instanceId.end());

                // Extrair o FriendlyName
                std::getline(jsonStream, line);
                std::wstring friendlyName = line.substr(line.find(L":") + 1);
                friendlyName.erase(remove(friendlyName.begin(), friendlyName.end(), L'"'), friendlyName.end());
                friendlyName.erase(remove(friendlyName.begin(), friendlyName.end(), L','), friendlyName.end());

                //Extrair o Status
                std::getline(jsonStream, line);
                std::wstring status = line.substr(line.find(L":") + 1);
                status.erase(remove(status.begin(), status.end(), L'"'), status.end());
                status.erase(remove(status.begin(), status.end(), L','), status.end());

                // Filtrar dispositivos BTHENUM e aplicar a palavra-chave
                if (status.find(L"OK") != std::wstring::npos && (keyword.empty() || instanceId.find(keyword) != std::wstring::npos || friendlyName.find(keyword) != std::wstring::npos)) {
                    devices.push_back({ instanceId, friendlyName, status }); // Adiciona o dispositivo à lista
                }
            }
        }
    }
	else if (option == 2)
	{
		while (std::getline(jsonStream, line)) {
			if (line.find(L"InstanceId") != std::wstring::npos) {
				// Extrair o InstanceId
				std::wstring instanceId = line.substr(line.find(L":") + 1);
				instanceId.erase(remove(instanceId.begin(), instanceId.end(), L'"'), instanceId.end());
				instanceId.erase(remove(instanceId.begin(), instanceId.end(), L','), instanceId.end());

				// Extrair o FriendlyName
				std::getline(jsonStream, line);
				std::wstring friendlyName = line.substr(line.find(L":") + 1);
				friendlyName.erase(remove(friendlyName.begin(), friendlyName.end(), L'"'), friendlyName.end());
				friendlyName.erase(remove(friendlyName.begin(), friendlyName.end(), L','), friendlyName.end());

				//Extrair o Status
				std::getline(jsonStream, line);
				std::wstring status = line.substr(line.find(L":") + 1);
				status.erase(remove(status.begin(), status.end(), L'"'), status.end());
				status.erase(remove(status.begin(), status.end(), L','), status.end());

				if (status.find(L"Error") != std::wstring::npos)
				{
					devices.push_back({ instanceId, friendlyName, status }); // Adiciona o dispositivo à lista
				}
			}
		}
	}
	else
	{
		std::wcout << L"Opção invalida." << std::endl;
	}


    return devices;
}

// Função para adicionar um comando de desinstalação ao Registro
void AddStartupUninstall(const std::wstring& deviceId) {
    HKEY hKey;
    std::wstring deviceId_ = splitByDoubleBackslash(deviceId);

    if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {

        // Monta o comando PowerShell para desinstalar o dispositivo
        std::wstring command = L"powershell.exe -Command \"Disable-PnpDevice -InstanceId '" + ReplaceEscapeSequences(deviceId_) + L"' -Confirm:$false\"";

        // Registra o comando para ser executado na inicialização
        RegSetValueEx(hKey, deviceId.c_str(), 0, REG_SZ, (const BYTE*)command.c_str(), (command.size() + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);
        std::wcout << L"Comando adicionado ao registro para desinstalar o dispositivo na inicializacao." << std::endl;
    } else {
        std::wcerr << L"Erro ao acessar o Registro: " << GetLastError() << std::endl;
    }
}

// Função para desinstalar um dispositivo imediatamente
void UninstallDevice(const std::wstring& deviceId) {
    std::wstring deviceId_ = splitByDoubleBackslash(deviceId);

    // Comando PowerShell para desinstalar o dispositivo
    std::wstring command = L"powershell.exe -Command \"Disable-PnpDevice -InstanceId '" + ReplaceEscapeSequences(deviceId_) + L"' -Confirm:$false\"";

    try {
        // Executa o comando e captura a saída
        std::wstring output = ExecutePowerShellCommand(command);
        std::wcout << L"Driver Desinstalado!\n" << output << std::endl;
    } catch (const std::runtime_error& e) {
        std::wcerr << L"Erro ao desinstalar o dispositivo: " << e.what() << std::endl;
    }
}

int main() {

    // Lista dispositivos BTHENUM
    std::vector<DeviceInfo> devices = ListDevices();
    bool option = 0x01;
    
    // Verifica se não há dispositivos encontrados
    if (devices.empty()) {
        std::wcout << L"Nenhum dispositivo encontrado." << std::endl;
        return 0;
    }

    // Exibe os dispositivos encontrados
    std::wcout << L"Dispositivos encontrados:\n";
    for (size_t i = 0; i < devices.size(); ++i) {
        std::wcout << i + 1 << L": " << devices[i].deviceName << devices[i].status << L"\n(ID:" << devices[i].deviceId << L")\n" << std::endl;
        if (devices[i].status.find(L"Error") != std::wstring::npos)
        {
			std::wcout << L"Deseja  restaurar o driver? (s/n): ";
			char choiceRestore;
			std::cin >> choiceRestore;
			if (choiceRestore == 's' || choiceRestore == 'S')
			{
				std::wstring deviceId_ = splitByDoubleBackslash(devices[i].deviceId);
				std::wstring command = L"powershell.exe -Command \"Enable-PnpDevice -InstanceId '" + ReplaceEscapeSequences(deviceId_) + L"' -Confirm:$false\"";
				std::wstring output = ExecutePowerShellCommand(command);
				std::wcout << L"Driver restaurado!\n" << output << std::endl;
				std::wcout << L"Removendo do registro...\n";

				HKEY hKey;
				if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
					RegDeleteValue(hKey, devices[i].deviceId.c_str());
					RegCloseKey(hKey);
					std::wcout << L"Entrada removida do registro." << std::endl;
				}
				else {
					std::wcerr << L"Erro ao acessar o Registro: " << GetLastError() << std::endl;
				}
			}
            option = 0x00;
        }
    }

    if (option)
    {
        while (1)
        {
            std::wcout << L"\nSelecione um dispositivo para desinstalar (ou digite 0 para sair): ";
            int choice;
            std::cin >> choice;

            auto intToBool = [](int choice) -> bool {
                return choice != 0;
                };

            if (!intToBool(choice)) {
                std::wcout << L"Escolha invalida." << std::endl;
                break;
            }

            // Validação da escolha do usuário
            /*if (choice > 0 && choice <= static_cast<int>(devices.size())) {
                const std::wstring& selectedDeviceId = devices[choice - 1].deviceId;
                AddStartupUninstall(selectedDeviceId); // Adiciona ao registro
                UninstallDevice(selectedDeviceId); // Desinstala o dispositivo
                
            }*/
            
            std::wcout << L"\nDeseja desinstalar outro drive? (s/n): ";
            char choiceFlag;
            std::cin >> choiceFlag;
            if (choiceFlag == 'n' || choiceFlag == 'N')
                break;
            else if (choiceFlag == 's' || choiceFlag == 'S')
            {
				system("cls");
                for (size_t i = 0; i < devices.size(); ++i) {
                    std::wcout << i + 1 << L": " << devices[i].deviceName << devices[i].status << L"\n(ID:" << devices[i].deviceId << L")\n" << std::endl;
                }
				
            }
        }
    }

    system("pause"); // Pausa a execução para visualização
    return 0;
}
