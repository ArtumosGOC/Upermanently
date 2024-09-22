## Sobre

O projeto "Uninstall Driver Permanently" fornece um script para "desinstalar"(desativa, não era a ideia inicial) drivers permanentemente do seu sistema Windows. Ele é especialmente útil para remover drivers problemáticos que continuam sendo reinstalados automaticamente pelo sistema operacional. O script permite listar os dispositivos presentes no sistema, selecionar um dispositivo específico e desinstalá-lo, além de adicionar um comando de desinstalação ao Registro para execução na inicialização.


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
    x64/Debug/Unintall\ Driver\ permanently.exe
    ```
    ` USE O ADMINISTRADOR `
3. Siga as instruções no terminal para listar e selecionar o dispositivo a ser desinstalado.

## Funções Principais

### Desinstalar Dispositivo

A função `UninstallDevice` desinstala um dispositivo dado seu ID.

### Adicionar Comando de Desinstalação ao Registro

A função `AddStartupUninstall` adiciona um comando de desinstalação ao Registro para execução na inicialização.

## Contribuição

1. Faça um fork do projeto
2. Crie uma nova branch (`git checkout -b feature/nova-feature`)
3. Commit suas mudanças (`git commit -am 'Adiciona nova feature'`)
4. Faça um push para a branch (`git push origin feature/nova-feature`)
5. Abra um Pull Request

## Licença

Este projeto está licenciado sob a licença MIT. Veja o arquivo LICENSE.txt para mais detalhes.