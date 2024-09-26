## Sobre

O projeto "Uninstall Driver Permanently" fornece um script para desativar drivers permanentemente do seu sistema Windows. Ele é especialmente útil para remover drivers problemáticos que continuam sendo reinstalados automaticamente pelo sistema operacional. O script permite listar os dispositivos presentes no sistema, selecionar um dispositivo específico e desativá-lo, além de adicionar um comando de desativação ao Registro para execução na inicialização.

## Pré-requisitos

Certifique-se de que você tem as seguintes dependências instaladas:

- Visual Studio 2022
- Windows SDK

## Instalação

1. Clone o repositório:
    ```sh
    git clone https://github.com/seu-usuario/Upermanently.git
    ```
2. Abra o projeto no Visual Studio 2022:
    ```sh
    cd Upermanently
    start Upermanently.sln
    ```

## Uso

Para desinstalar um driver, siga os passos abaixo:

1. Compile o projeto no Visual Studio 2022.
2. Execute o executável gerado:
    ```sh
    x64/Debug/permanently.exe
    ```
    `USE O ADMINISTRADOR`
3. Siga as instruções no terminal para listar e selecionar o dispositivo a ser desinstalado.

## Funções Principais

### Desinstalar Dispositivo

A função `UninstallDevice` desinstala um dispositivo dado seu ID.

### Adicionar Comando de Desinstalação ao Registro

A função `AddStartupUninstall` adiciona um comando de desinstalação ao Registro para execução na inicialização.

### Listar Dispositivos

A função `ListDevices` lista os dispositivos presentes no sistema.

### Substituir Substring

A função `replaceSubstring` substitui uma substring por outra em uma string.

### Substituir Sequências de Escape

A função `ReplaceEscapeSequences` substitui sequências de escape em uma string.

### Converter Texto para Maiúsculas

A função `TextUpper` converte uma string para maiúsculas.

### Substituir "\\" por "\"

A função `splitByDoubleBackslash` substitui ocorrências de "\\" por "\" em uma string.

### Criar Arquivo de Inicialização

A função `CreateStartupBat` cria um arquivo .bat para inicialização.

## Contribuição

1. Faça um fork do projeto
2. Crie uma nova branch `git checkout -b feature/nova-feature`
3. Commit suas mudanças `git commit -am 'Adiciona nova feature'`
4. Faça um push para a branch `git push origin feature/nova-feature`
5. Abra um Pull Request

## Licença

Este projeto está licenciado sob a licença MIT. Veja o arquivo LICENSE.txt para mais detalhes.