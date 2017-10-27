/**
 * SGN - Robótica Educacional
 * JUN-2017
 *
 * Projeto: Exemplo de datalogger com ESP8266
 *
 * Resumo: Transferir para uma planilha do Google Drive os dados
 * de temperatura, umidade recebidos por um comando GET.
 *
 * Link para executar este script:
 * https://script.google.com/macros/s/SCRIPT_ID/exec?sensor_id=Teste&temp=25&hum=75
 */

function doGet(data) {
  
  var sensor_id = "";  // Nome/local da estação de telemetria
  var temp = "";       // Variável para armazenar os dados de temperatura
  var hum = "";        // Variável para armazenar os dados de umidade
  
  try
  {
    /** Preenche os dados em caso de receber requisição vazia. Usado para testes */
    if(data == null)
    {
      data = {};
      data.parameters = {
        sensor_id:"ID3",
        temp: "25",
        hum: "75",
      }
    }
    
    /** Atribui os valores obtidos às variáveis */
    sensor_id = data.parameters.sensor_id;
    temp = data.parameters.temp;
    hum = data.parameters.hum;
    
    /** Executa a função que transfere os dados para o script */
    storeData(sensor_id, temp, hum);
    
    return ContentService.createTextOutput("Wrote:\n  id: " + sensor_id + "\n  temp: " + temp + "\n  hum:" + hum);
      
  }
  catch(error)
  {
    return ContentService.createTextOutput("oops...." + error.message);
  }
}

function storeData(sensor_id, temp, hum)
{
  /* Armazena a URL da planilha */
  var sheetUrl = "https://docs.google.com/spreadsheets/d/SHEET_ID/edit";
  var ss = SpreadsheetApp.openByUrl(sheetUrl);
  
  /** Executa a função que transfere os dados para o script */
  storeOnMainSheet(ss, sensor_id, temp, hum);
  storeOnSpecificSheet(ss, sensor_id, temp, hum)
 
}

function storeOnMainSheet(spreadsheet, sensor_id, temp, hum)
{
   try
  {      
    /** Abre a planilha e procura a página selecionada */
    writeSensorData(spreadsheet, "Temperatura", sensor_id, temp);
    writeSensorData(spreadsheet, "Umidade", sensor_id, hum);
  }
  catch(error) {
    var err = JSON.stringify(error);
    Logger.log(JSON.stringify(error));
  }
}

function writeSensorData(spreadsheet, sheetName, sensor_id, value)
{
  try
  {
    /** Procura a página correta através do nome*/
    var sheet = spreadsheet.getSheetByName(sheetName);
    
    /** 
      * Determina o número de colunas. Assim saberemos em 
      * quantas colunas devemos procurar nosso dado 
      */
    var lastCol = sheet.getLastColumn();
    
    /** Cria um vetor com os títulos das colunas */
    var colTitles = sheet.getSheetValues(1, 1, 1, lastCol);
    
    var col = 1;        // Variável utilizada para a contagem de colunas
    var targetCol = ""; // Variável que armazena a coluna alvo. Inicializa nulo
    
    
    for(col = 0; col < lastCol; col++)
    {
      // Procura em todas as colunas aquela que corresponde ao identificador do sensor
      if(colTitles[0][col] == sensor_id)
      {
        // Ao encontrar a coluna, o valor dela é salvo.
        targetCol = col;
        break;
      }
    }
    
    if(targetCol == "")
    {
      // Se a coluna não foi encontrada, o valor ainda é nulo.
      // Logo, se o valor for nulo precisamos criar a coluna.
      targetCol = col + 1;
      sheet.getRange(1, targetCol).setValue(sensor_id);
    }
    else
    {
      targetCol = targetCol + 1;
    }
    
    var date = new Date();   // Determina a data e hora enviada pelo sensor
    var targetRow = sheet.getLastRow() + 1;  // Verifica o número de linhas em que a busca será feita
    
    // Armazena os valores na planilha
    sheet.getRange(targetRow, targetCol).setValue(value);
    sheet.getRange(targetRow, 1).setValue(date);
  }
  catch(error) {
    var err = JSON.stringify(error);
    Logger.log(JSON.stringify(error));
  }
}

function storeOnSpecificSheet(spreadsheet, sensor_id, temp, hum)
{
    try
  {    
    /** Procura a página selecionada */
    var dataLogSheet = spreadsheet.getSheetByName(sensor_id);
    
    /** Ser a página ainda não existe, ela é criada */
    if(dataLogSheet == null)
    {
      /** Cria a página com nome conhecido para depois renomeá-la. Criar direto com o nome causava um bug */
      spreadsheet.insertSheet("NewReservedName");
      dataLogSheet = spreadsheet.getSheetByName("NewReservedName");
      dataLogSheet.setName(sensor_id);    
      
      dataLogSheet.getRange("A1").setValue("Dispositivo ");
      dataLogSheet.getRange("B1").setValue(sensor_id);
      dataLogSheet.getRange("A2").setValue("Data/Hora");
      dataLogSheet.getRange("B2").setValue("Temperatura");
      dataLogSheet.getRange("C2").setValue("Umidade");
      
    }
    
    /** Obtém a última linha preenchida) */
    var row = dataLogSheet.getLastRow() + 1;
    
    /** Preenche os dados */
    dataLogSheet.getRange("A" + row).setValue(new Date()); // Data/Hora
    dataLogSheet.getRange("B" + row).setValue(temp); // Temperatura
    dataLogSheet.getRange("C" + row).setValue(hum);  // Umidade

  }
  catch(error) {
    Logger.log(JSON.stringify(error));
  }
}
