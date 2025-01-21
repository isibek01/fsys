/*
Description: extended app get config file.
Parameters:
Author: Fagor Automation S. Coop.
Version: 1.0
*/

//Uncomment next line to debug JS
//debugger;

// Global defines
var fso = new ActiveXObject("Scripting.FileSystemObject"); 
var WshShell = WScript.CreateObject ("WScript.Shell");
var doc = new ActiveXObject("msxml2.DOMDocument");
var objArgs = WScript.Arguments;
var path = "";
var fileName = "";
var fullFileName = "";

if(objArgs.length > 0)
	fileName = objArgs(0);					//El que se le pasa como argumento.
else
	fileName = "optitwinConfig.xml"; 	//Por defecto

if (fileName.indexOf(':') > -1)
	fullFileName = fileName; 				//Absolute Path. No more changes.
else
	fullFileName = fso.GetAbsolutePathName(path) + "\\" + fileName;	// Relative path. Get the absolute one

//Parse xml file
doc.async = false;
doc.resolveExternals = false;
doc.validateOnParse = false;
// Load an XML file into the DOM instance.
doc.load(fullFileName);

// Leo el fichero salvo que haya error
if (doc.parseError.errorCode != 0) {
	var myErr = doc.parseError;
    WScript.Echo("Error " + myErr.reason);
    WScript.Quit(1);
}
else {
	WScript.echo("#####");  //5 estrellas para separar zonas, aqui, empezar
	
	var paramsTag = doc.getElementsByTagName("params"); 
	var attribute = paramsTag[0].getAttribute("algorithmVersion");
	WScript.echo("algorithmVersion = '"+ attribute + "'");
	attribute = paramsTag[0].getAttribute("algorithmEnable");
	WScript.echo("algorithmEnable = '"+ attribute + "'");
	attribute = paramsTag[0].getAttribute("closeFile");
	WScript.echo("closeFile = '"+ attribute + "'");
	attribute = paramsTag[0].getAttribute("warningMessage");
	WScript.echo("warningMessage = '"+ attribute + "'");
	attribute = paramsTag[0].getAttribute("optitwinPort");
	WScript.echo("optitwinPort = '"+ attribute + "'");
	attribute = paramsTag[0].getAttribute("folderCSV");
	// Add trailing backslash if necessary
	if (attribute && attribute.charAt(attribute.length - 1) !== "\\") {
		attribute += "\\";
	}
	WScript.echo("folder = '" + attribute + "'");
	
	var paramArray = doc.getElementsByTagName("param");
	WScript.echo("TOTAL_PARAMS = " + paramArray.length);
	for (i=0; i<paramArray.length; i++) {
		attribute = myGetElementsByTagName(paramArray[i], "varName")
		if (attribute != null && attribute.length > 0) {
			WScript.Echo("varName = '" + attribute + "'");
			attribute = myGetElementsByTagName(paramArray[i], "factor")
			if(attribute != null && attribute.length > 0)
				WScript.Echo("factor = " + attribute + "");
			else
				WScript.Echo("factor = 1");
		}
	}
	WScript.echo("EOF 0");  //EOF, terminar
}

function myGetElementsByTagName(param, tagName) {
	//Nos aseguramos de que la etiqueta no esta vacia. Si se intenta leer una etiqueta vacia NULL POINTER. ERROR:
	if(param.getElementsByTagName(tagName).length == 0)
		var varAux = null;
	else
		var varAux = param.getElementsByTagName(tagName)[0].text;		
	return (varAux);
}