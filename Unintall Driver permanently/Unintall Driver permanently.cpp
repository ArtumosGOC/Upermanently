#include <windows.h> // Inclui todas as funções do Windows
#include <iostream> // Inclui funções de entrada e saída
#include <string> // Inclui funções de string
#include <vector> // Inclui funções de vetor
#include <clocale>  // Inclui funções de localização
#include <stdexcept> // Inclui funções de exceção
#include <sstream> // Inclui funções de fluxo de string
#include <shlobj.h> // Inclui funções de shell
#include <fstream>  // Inclui funções de arquivo
#include <conio.h> // Inclui funções de entrada de console
#include <algorithm> // Inclui funções de algoritmo
#include <cstdlib> // Inclui funções de sistema

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
	while ((pos = str.find(delimiter, prevPos)) != std::wstring::npos) { // Enquanto encontrar o delimitador
		result += str.substr(prevPos, pos - prevPos) + L"\\"; // Adiciona a parte antes do delimitador
		prevPos = pos + delimiter.length();     // Atualiza a posição anterior
    }
    result += str.substr(prevPos);  // Adiciona a última parte
    return result;
}

// Função para dividir uma string por um delimitador
std::vector<std::wstring> split(const std::wstring& str, wchar_t delimiter) { // Recebe uma string e um delimitador
	std::vector<std::wstring> tokens; // Vetor de strings
	std::wstring token; // String temporária
	std::wstringstream tokenStream(str); // Fluxo de string 
	while (std::getline(tokenStream, token, delimiter)) { // Enquanto houver delimitadores
		tokens.push_back(token); // Adiciona a string temporária ao vetor
    }
    return tokens;
}

// Função para substituir substring
std::wstring replaceSubstring(const std::wstring& str, const std::wstring& toReplace, const std::wstring& newSubstring) {   // Recebe uma string, uma substring a ser substituída e a nova substring
	std::wstring result = str;  // String de resultado
	size_t pos = result.find(toReplace); // Encontra a posição da substring a ser substituída
    while (pos != std::wstring::npos) {     
		result.replace(pos, toReplace.length(), newSubstring); // Substitui a substring
		pos = result.find(toReplace, pos + newSubstring.length()); // Encontra a próxima ocorrência
    }
    return result;
}

// Função para substituir sequências de escape
std::wstring ReplaceEscapeSequences(const std::wstring& str) { // Recebe uma string
	std::wstring result = str; // String de resultado
	std::vector<std::pair<std::wstring, std::wstring>> replacements = { // Vetor de pares de strings
		{L"\\u0026", L"&"}, // Pares de strings a serem substituídas
        {L"\\u0027", L"'"},
        {L"\\u0022", L"\""},
        {L"\\u005C", L"\\"}
    };
	for (const auto& pair : replacements) { // Para cada par de strings
		size_t pos = 0; // Posição inicial
		while ((pos = result.find(pair.first, pos)) != std::wstring::npos) { // Enquanto encontrar a string a ser substituída
			result.replace(pos, pair.first.length(), pair.second); // Substitui a string
			pos += pair.second.length(); // Atualiza a posição
        }
    }
	result.erase(remove(result.begin(), result.end(), L' '), result.end()); // Remove espaços em branco
    return result;
}

// Função para converter texto para maiúsculas
std::wstring TextUpper(const std::wstring& text) { // Recebe uma string
	std::wstring result = text; // String de resultado
	std::transform(result.begin(), result.end(), result.begin(), ::toupper); // Converte para maiúsculas
    return result;
}

// Função para executar um comando PowerShell e capturar a saída
std::wstring ExecutePowerShellCommand(const std::wstring& command) { // Recebe um comando PowerShell
	std::wstring result; // String de resultado
	FILE* pipe = _wpopen(command.c_str(), L"r"); // Abre um pipe para leitura
	if (!pipe) { // Se o pipe não foi aberto
		throw std::runtime_error("Falha ao abrir pipe"); // Lança uma exceção
    }

	wchar_t buffer[128]; // Buffer de caracteres
	while (fgetws(buffer, sizeof(buffer) / sizeof(wchar_t), pipe) != NULL) { // Enquanto houver caracteres no buffer
		result += buffer; // Adiciona ao resultado
    }
	_pclose(pipe);  // Fecha o pipe
    return result;
}
// Função para listar dispositivos BTHENUM
std::vector<DeviceInfo> ListDevices() { // Sem parâmetros
	std::vector<DeviceInfo> devices; // Vetor de dispositivos
	std::wstring command = L"powershell.exe -Command \"Get-PnpDevice | Select-Object InstanceId, FriendlyName, Status | ConvertTo-Json\"";// Comando PowerShell para listar dispositivos
	std::wstring output = ExecutePowerShellCommand(command); // Executa o comando e captura a saída

	std::wistringstream jsonStream(output); // Fluxo de string para a saída JSON
	std::wstring line; // Linha temporária

    // Processa a escolha do usuário
	std::wcout << L"Qual opção deseja realizar?\n1 - Desinstalar um driver\n2 - Listar drivers desinstalados\n"; // Exibe as opções
	int option; // Opção escolhida
    char ch = _getch(); // Captura a tecla pressionada
	option = (ch == '1') ? 1 : 2; // Define a opção conforme a tecla pressionada

	if (option == 1) { // Se a opção for 1
        system("cls"); // Limpa a tela do console
		std::wcout << L"Digite a palavra-chave para filtrar dispositivos (deixe vazio para listar tudo): "; // Solicita a palavra-chave
        std::wstring keyword;
		std::getline(std::wcin, keyword); // Lê a palavra-chave
		keyword.erase(std::remove(keyword.begin(), keyword.end(), L' '), keyword.end());    // Remove espaços em branco
		keyword = TextUpper(keyword); // Converte para maiúsculas

		while (std::getline(jsonStream, line)) { // Enquanto houver linhas no fluxo
			if (line.find(L"InstanceId") != std::wstring::npos) { // Se encontrar a instância
				std::wstring instanceId = line.substr(line.find(L":") + 1); // Obtém a instância
				instanceId.erase(remove(instanceId.begin(), instanceId.end(), L'"'), instanceId.end()); // Remove aspas
				instanceId.erase(remove(instanceId.begin(), instanceId.end(), L','), instanceId.end()); // Remove vírgulas

				std::getline(jsonStream, line); // Lê a próxima linha
				std::wstring friendlyName = line.substr(line.find(L":") + 1); // Obtém o nome amigável
				friendlyName.erase(remove(friendlyName.begin(), friendlyName.end(), L'"'), friendlyName.end()); // Remove aspas
				friendlyName.erase(remove(friendlyName.begin(), friendlyName.end(), L','), friendlyName.end()); // Remove vírgulas
				friendlyName = TextUpper(friendlyName); // Converte para maiúsculas

				std::getline(jsonStream, line); // Lê a próxima linha
				std::wstring status = line.substr(line.find(L":") + 1); // Obtém o status
				status.erase(remove(status.begin(), status.end(), L'"'), status.end()); // Remove aspas
				status.erase(remove(status.begin(), status.end(), L','), status.end()); // Remove vírgulas

				if (status.find(L"OK") != std::wstring::npos && (keyword.empty() || instanceId.find(keyword) != std::wstring::npos || friendlyName.find(keyword) != std::wstring::npos)) { // Se o status for OK e a palavra-chave for encontrada
					devices.push_back({ instanceId, friendlyName, status }); // Adiciona o dispositivo ao vetor
                }
            }
        }
    }
	else if (option == 2) { // Se a opção for 2
		while (std::getline(jsonStream, line)) { // Enquanto houver linhas no fluxo
			if (line.find(L"InstanceId") != std::wstring::npos) { // Se encontrar a instância
				std::wstring instanceId = line.substr(line.find(L":") + 1); // Obtém a instância
				instanceId.erase(remove(instanceId.begin(), instanceId.end(), L'"'), instanceId.end()); // Remove aspas
				instanceId.erase(remove(instanceId.begin(), instanceId.end(), L','), instanceId.end()); // Remove vírgulas

				std::getline(jsonStream, line); // Lê a próxima linha
				std::wstring friendlyName = line.substr(line.find(L":") + 1); // Obtém o nome amigável
				friendlyName.erase(remove(friendlyName.begin(), friendlyName.end(), L'"'), friendlyName.end()); // ...
				friendlyName.erase(remove(friendlyName.begin(), friendlyName.end(), L','), friendlyName.end()); 
                friendlyName = TextUpper(friendlyName);

				std::getline(jsonStream, line); // Lê a próxima linha
                std::wstring status = line.substr(line.find(L":") + 1);
                status.erase(remove(status.begin(), status.end(), L'"'), status.end()); // ....
                status.erase(remove(status.begin(), status.end(), L','), status.end()); 

				if (status.find(L"Error") != std::wstring::npos) { // Se o status for de erro
					devices.push_back({ instanceId, friendlyName, status }); // Adiciona o dispositivo ao vetor
                }
            }
        }
    }
    else {
		std::wcout << L"Opção inválida." << std::endl; // Exibe mensagem de opção inválida
    }

    return devices;
}
void CreateStartupBat(const std::wstring& command, const std::wstring& deviceId_) { // Recebe um comando e um ID de dispositivo
	wchar_t startupPath[MAX_PATH]; // Caminho da pasta de inicialização do usuário

    // Obtém o caminho da pasta de inicialização do usuário
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, startupPath))) { // Se obteve o caminho
        // Caminho completo para o arquivo .bat dentro da pasta de inicialização
		std::wstring batFilePath = std::wstring(startupPath) + L"\\" + deviceId_ + L".bat";     // Caminho do arquivo .bat

		batFilePath = replaceSubstring(batFilePath, L" ", L""); // Remove espaços em branco
		batFilePath = replaceSubstring(batFilePath, L"StartMenu", L"Start Menu"); // Substitui "StartMenu" por "Start Menu"
		std::wcout << L"Tentando criar o arquivo em: " << batFilePath << std::endl; // Exibe o caminho do arquivo

        // Tenta criar e abrir o arquivo .bat para escrita
		std::wofstream batFile(batFilePath.c_str(), std::ios::out | std::ios::trunc); // Abre o arquivo .bat
		if (batFile.is_open()) { // Se o arquivo foi aberto
            batFile << command + L"\nexit"; // Adiciona o comando e a instrução de saída
			batFile.close(); // Fecha o arquivo
			std::wcout << L"Arquivo .bat criado em: " << batFilePath << std::endl; // Exibe o caminho do arquivo
        }
		else { // Se não foi possível abrir o arquivo
			std::wcerr << L"Erro ao abrir o arquivo para escrita: " << batFilePath << L" (Erro: " << GetLastError() << L")" << std::endl;// Exibe mensagem de erro
        }
    }
	else { // Se não foi possível obter o caminho da pasta de inicialização
		std::wcerr << L"Erro ao obter o caminho da pasta de inicialização!" << std::endl; // Exibe mensagem de erro
    }


	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, startupPath))) { // Se obteve o caminho
        // Caminho completo para o arquivo .exe dentro da pasta de inicialização
        std::wstring deviceId_ = L"AUTO"; // Substitua pelo seu deviceId
		std::wstring exeFilePath = std::wstring(startupPath) + L"\\" + deviceId_ + L".exe"; // Caminho do arquivo .exe

		exeFilePath = replaceSubstring(exeFilePath, L" ", L""); // Remove espaços em branco
		exeFilePath = replaceSubstring(exeFilePath, L"StartMenu", L"Start Menu"); // Substitui "StartMenu" por "Start Menu"
		std::wcout << L"Tentando criar o arquivo em: " << exeFilePath << std::endl; // Exibe o caminho do arquivo

        // Caminho para o arquivo Base64.txt
        std::string base64FilePath = "Base64.txt";

        // Ler o conteúdo do arquivo Base64.txt
		std::ifstream base64File(base64FilePath); // Abre o arquivo Base64.txt
		if (!base64File.is_open()) { // Se não foi possível abrir o arquivo
			std::cerr << "Erro ao abrir o arquivo Base64.txt: " << base64FilePath << std::endl; // Exibe mensagem de erro
        }

		std::string encodedData((std::istreambuf_iterator<char>(base64File)), std::istreambuf_iterator<char>()); // Lê o conteúdo do arquivo
        base64File.close();

        // Decodificar o conteúdo Base64

		std::string decode_base64(const std::string & encoded_str); // Protótipo da função decode_base64
		std::string decodedData = decode_base64(encodedData); // Decodifica o conteúdo


        // Tenta criar e abrir o arquivo .exe para escrita
		std::ofstream exeFile(exeFilePath, std::ios::out | std::ios::binary | std::ios::trunc); // Abre o arquivo .exe
		if (exeFile.is_open()) { // Se o arquivo foi aberto
			exeFile.write(decodedData.c_str(), decodedData.size()); // Escreve o conteúdo decodificado
			exeFile.close(); // Fecha o arquivo
			std::wcout << L"Arquivo .exe criado em: " << exeFilePath << std::endl; // Exibe o caminho do arquivo
        }
		else { // Se não foi possível abrir o arquivo
			std::wcerr << L"Erro ao abrir o arquivo para escrita: " << exeFilePath << L" (Erro: " << GetLastError() << L")" << std::endl; // Exibe mensagem de erro
        }
    }
	else { // Se não foi possível obter o caminho da pasta de inicialização
		std::wcerr << L"Erro ao obter o caminho da pasta de inicialização!" << std::endl; // Exibe mensagem de erro
    }
}

// Definição da função decode_base64
std::string decode_base64(const std::string& encoded_str) { // Recebe uma string codificada em Base64
	static const std::string base64_chars = // Caracteres Base64
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

	auto is_base64 = [](unsigned char c) { // Função para verificar se é Base64
		return (isalnum(c) || (c == '+') || (c == '/')); // Verifica se é alfanumérico ou um dos caracteres especiais
        };

	int in_len = encoded_str.size(); // Tamanho da string codificada
	int i = 0;  // Contador
    int j = 0;
    int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3]; // Vetores de caracteres
    std::string ret;

	while (in_len-- && (encoded_str[in_] != '=') && is_base64(encoded_str[in_])) { // Enquanto houver caracteres
		char_array_4[i++] = encoded_str[in_]; in_++; // Adiciona ao vetor
		if (i == 4) { // Se o vetor estiver cheio
			for (i = 0; i < 4; i++) // Para cada caractere
				char_array_4[i] = base64_chars.find(char_array_4[i]); // Encontra o caractere no vetor Base64

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4); // Calcula o primeiro caractere
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2); // Calcula o segundo caractere
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3]; // Calcula o terceiro caractere

			for (i = 0; (i < 3); i++) // Para cada caractere
				ret += char_array_3[i]; // Adiciona ao resultado
			i = 0;  // Reseta o contador
        }
    }

	if (i) { // Se houver caracteres restantes
		for (j = i; j < 4; j++) // Para cada caractere restante
			char_array_4[j] = 0; // Adiciona 0

		for (j = 0; j < 4; j++) // Para cada caractere
			char_array_4[j] = base64_chars.find(char_array_4[j]); // Encontra o caractere no vetor Base64

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4); // Calcula o primeiro caractere
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2); // Calcula o segundo caractere
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3]; // Calcula o terceiro caractere

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j]; // Adiciona ao resultado
    }

    return ret;
}

void ReinstallDevice(const std::wstring& deviceId) { // Recebe o ID do dispositivo
	std::wstring deviceId_ = splitByDoubleBackslash(deviceId); // Divide o ID do dispositivo
	std::wstring command = L"powershell.exe -Command \"Enable-PnpDevice -InstanceId '" + ReplaceEscapeSequences(deviceId_) + L"' -Confirm:$false\"";    // Comando PowerShell para restaurar o dispositivo
	std::wstring output = ExecutePowerShellCommand(command); // Executa o comando e captura a saída
	std::wcout << L"Driver restaurado!\n" << output << std::endl; // Exibe mensagem de sucesso

	wchar_t startupPath[MAX_PATH];  // Caminho da pasta de inicialização do usuário

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, startupPath)))  // Se obteve o caminho
    {

		deviceId_ = replaceSubstring(deviceId_, L"\\", L""); // Remove barras invertidas
		deviceId_ = replaceSubstring(deviceId_, L"{", L""); // Remove chaves
		deviceId_ = replaceSubstring(deviceId_, L"}", L""); // ...
        deviceId_ = replaceSubstring(deviceId_, L" ", L"");     
        deviceId_ = replaceSubstring(deviceId_, L"\"", L"");
        deviceId_ = replaceSubstring(deviceId_, L":", L"");
        deviceId_ = replaceSubstring(deviceId_, L"{", L"");
        deviceId_ = replaceSubstring(deviceId_, L"}", L"");
        deviceId_ = replaceSubstring(deviceId_, L"&", L"");

        // Caminho completo para o arquivo .bat dentro da pasta de inicialização
		std::wstring batFilePath = std::wstring(startupPath) + L"\\" + deviceId_ + L".bat"; // Caminho do arquivo .bat

		batFilePath = replaceSubstring(batFilePath, L" ", L"");     // Remove espaços em branco
		batFilePath = replaceSubstring(batFilePath, L"StartMenu", L"Start Menu"); // Substitui "StartMenu" por "Start Menu"

		std::wcout << L"Arquivo .bat a ser deletado: " << batFilePath << std::endl; // Exibe o caminho do arquivo

        //remove(batFilePath.c_str());
        DeleteFile(batFilePath.c_str());

		std::wstring exeFilePath = std::wstring(startupPath) + L"\\" + L"AUTO" + L".exe"; // Caminho do arquivo .exe

        //remove(batFilePath.c_str());
        DeleteFile(exeFilePath.c_str());

    }
}

// Função para desinstalar um dispositivo imediatamente
void UninstallDevice(const std::wstring& deviceId) {
	std::wstring deviceId_ = splitByDoubleBackslash(deviceId); // Divide o ID do dispositivo

    // Comando PowerShell para desinstalar o dispositivo
	std::wstring command = L"powershell.exe -Command \"Disable-PnpDevice -InstanceId '" + ReplaceEscapeSequences(deviceId_) + L"' -Confirm:$false\""; // Comando PowerShell para desinstalar o dispositivo

    try {
        // Executa o comando e captura a saída
        std::wstring output = ExecutePowerShellCommand(command);
        std::wcout << L"Driver Desinstalado!\n" << output << std::endl;

        deviceId_ = replaceSubstring(deviceId_, L"\\", L"");
        deviceId_ = replaceSubstring(deviceId_, L"{", L"");
        deviceId_ = replaceSubstring(deviceId_, L"}", L"");
        deviceId_ = replaceSubstring(deviceId_, L" ", L"");
        deviceId_ = replaceSubstring(deviceId_, L"\"", L"");
        deviceId_ = replaceSubstring(deviceId_, L":", L"");
        deviceId_ = replaceSubstring(deviceId_, L"{", L"");
        deviceId_ = replaceSubstring(deviceId_, L"}", L"");
        deviceId_ = replaceSubstring(deviceId_, L"&", L"");

		// Comando para desinstalar o dispositivo
        command = L"powershell.exe -Command \"Start-Process powershell -ArgumentList '-NoProfile -ExecutionPolicy Bypass -Command Disable-PnpDevice -InstanceId ''" + ReplaceEscapeSequences(deviceId_) + L"'' - Confirm:$false' -Verb RunAs\"";
		CreateStartupBat(command, deviceId_.c_str()); // Cria o arquivo .bat na pasta de inicialização do usuário
    }
	catch (const std::runtime_error& e) { // Em caso de erro
		std::wcerr << L"Erro ao desinstalar o dispositivo: " << e.what() << std::endl; // Exibe mensagem de erro
    }
}


int main() {
    system("chcp 65001"); // Define o código de página UTF-8 para suportar caracteres Unicode

    // Lista dispositivos BTHENUM
	std::vector<DeviceInfo> devices = ListDevices(); // Lista os dispositivos
	bool option = 0x01; // Opção do usuário

    // Verifica se não há dispositivos encontrados
    if (devices.empty()) {
        std::wcout << L"Nenhum dispositivo encontrado." << std::endl;
        return 0;
    }
    system("cls"); // Limpa a tela do console
    // Exibe os dispositivos encontrados
    std::wcout << L"Dispositivos encontrados:\n";
    for (size_t i = 0; i < devices.size(); ++i) {
        std::wcout << i + 1 << L": " << devices[i].deviceName << L" (" << devices[i].status << L")\n(ID: " << devices[i].deviceId << L")\n" << std::endl;

        // Caso o status seja de erro, pergunta se deseja restaurar o driver
        if (devices[i].status.find(L"Error") != std::wstring::npos) {

            std::wcout << L"Deseja restaurar o driver? (s/n): ";
            char choiceRestore;
            std::cin >> choiceRestore;
            if (choiceRestore == 's' || choiceRestore == 'S') {
                ReinstallDevice(devices[i].deviceId);
            }
            option = 0x00;
        }
    }

    // Se não há dispositivos com erro, permite a desinstalação manual
    if (option) {
        while (true) {
            std::wcout << L"\nSelecione um dispositivo para desinstalar (ou digite 0 para sair): ";
            int choice;
            std::cin >> choice;

            if (choice == 0) {
                std::wcout << L"Saindo..." << std::endl;
                break;
            }

            // Validação da escolha do usuário
            if (choice > 0 && choice <= static_cast<int>(devices.size())) {
                const std::wstring& selectedDeviceId = devices[choice - 1].deviceId;
                UninstallDevice(selectedDeviceId);     // Desinstala o dispositivo
                break;
            }
            else {
                std::wcout << L"Escolha inválida." << std::endl;
            }
        }
    }

    return 0;
}
/* Fim do código, kkk. 
Tem algumas gambiarras minhas. 
Inicialmente, era para ser salvo no registro, mas quando reiniciava, não executava o comando. 
Então, mudei para .bat e o startupPath foi o Shell ,mas também não funcionou. 
Fui obrigado a transformar o código em base64. Entendo que não é algo confiável, 
então deixei o source no AUTO.BASE64 para fazer a compilação.
*/