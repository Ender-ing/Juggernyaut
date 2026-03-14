/**
 * 
 * Take care of Omniarium extension commands!
 * 
**/

// Imports
import * as vscode from "vscode";

// Constants
const DOCS_URL = 'https://ender.ing/docs/juggernyaut/';

// Message test
export const docs = vscode.commands.registerCommand('juggernyaut.docs', () => {
    const url = vscode.Uri.parse(DOCS_URL);
    vscode.env.openExternal(url);
});