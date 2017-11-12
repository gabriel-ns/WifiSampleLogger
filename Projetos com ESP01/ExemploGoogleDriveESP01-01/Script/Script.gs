/**
 * SGN - Robótica Educacional
 * OUT-2017
 *
 * Projeto: Exemplo de datalogger com ESP01 e Arduino UNO
 *
 * Resumo: Transferir para uma planilha do Google Drive os dados
 * recebidos por um comando GET.
 *
 * Link para executar este script:
 * https://script.google.com/macros/s/AKfycbwaBrPqQS3_8Wk7sMMWiUXttwKbWHDmdksfZ1ZbSKhlnkKppAmr/exec?value=100
 */

/* doGet: Função chamada numa requisição GET */
function doGet(data) {
  try
  {
    var value = ""; 
    /* Armazena o valor do parâmetro em uma variável*/
    value = data.parameters.value;
        
    /* Armazena a URL da planilha */
    var sheetUrl = "https://docs.google.com/spreadsheets/d/1p85abV67rwFLPr9L21Uc0WW8wyrEItwaJ_F8BIUiEWs/edit";
    /* Abre a planilha */    
    var spreadsheet = SpreadsheetApp.openByUrl(sheetUrl);
        
    /** Procura a página selecionada */
    var dataLogSheet = spreadsheet.getSheetByName("Datalog");
        
    /** Obtém a última linha preenchida) */
    var row = dataLogSheet.getLastRow() + 1;
    
    /** Preenche os dados */
    dataLogSheet.getRange("A" + row).setValue(new Date()); // Data/Hora
    dataLogSheet.getRange("B" + row).setValue(value); // valor

    return ContentService.createTextOutput("Wrote:\n  value: " + value );
      
  }
  catch(error)
  {
    var err = "oops...." + error.message;
    return ContentService.createTextOutput("oops...." + error.message);
  }
}


