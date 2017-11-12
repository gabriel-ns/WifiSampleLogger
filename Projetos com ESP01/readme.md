# Projetos com ESP01
Abaixo estão descritos os projetos contidos neste repositório.

## Exemplos 01 e 02
#### Integração do ESP01 ao Google Drive utilizando o Arduino UNO
Este projeto é desenvolvido tanto para o Platformio+Eclipse quanto para a IDE padrão do Arduino. Os projetos estão nas pastas:

* ExemploGoogleDriveESP01-01, para Platformio+Eclipse
* ExemploGoogleDriveESP01-02, para Arduino IDE

Para este projeto, utiliza-se o Arduino UNO em combinação com a placa ESP01. O firmware para a placa ESP01 é o de fábrica, que transforma ele num modem wifi que opera através de comandos AT, conectado ao Arduino UNO através da serial. A porta serial utilizada é feita pela biblioteca SoftwareSerial.

Ao baixar o repositório, navegue até a pasta com sua versão de preferência.

##### Página da Web para o servidor
Na subpasta Webpage está a descrição HTML da página utilizada para a interface do servidor web armazenado no Arduino UNO.

##### Script do Google Scripts
Na subpasta Script está o código para o Google Scripts que realiza a tarefa de receber os dados e colocá-los na planilha.

Neste script, é necessário alterar a URL para a planilha (linhas 23) na variável "sheetUrl", sendo que esta planilha deve ser aquela que foi criada no seu usuário do Google Drive. Para auxilílio no uso e testes, é recomendável colocar o link para o próprio script nos comentários do início do arquivo.

##### Arduino
Para começar, basta abrir o projeto com a IDE adequada.

No caso da IDE do Arduino, basta a configurar o projeto para o Arduino UNO e escolher a porta onde ele está localizado (para Arduino IDE).
No caso do Eclipse, basta clicar em File>Import>Existing projects to Workspace e selecionar a pasta do projeto. Para carregar o código, basta clicar em Platformio:Upload, localizado em Build Targets no Project Explorer (aba à esquerda).

A porta serial padrão do Arduino é utilizada para enviar dados de debug, como operações realizadas, comandos enviados, IP, etc, enquanto a comunicação com o ESP01 é feita através da biblioteca Software Serial. O projeto já entrega a infraestrutura necessária para a comunicação entre o ESP01 e o Arduino.

#### Montagem do hardware

A montagem básica utilizada para os exemplos citados está na imagem abaixo.
![Montagem](https://static.wixstatic.com/media/ecfc04_e844a351653f4917a51ccba903dbbcc4~mv2.png/v1/fill/w_630,h_249,al_c,usm_2.00_1.00_0.00/ecfc04_e844a351653f4917a51ccba903dbbcc4~mv2.png)