/**
 *
 * Start the extension!
 *
**/

// Imports
import * as vscode from "vscode";
import * as commands from "./commands";

// Lightweight extension activation! (for web browsers!)
export function lightweightActivation (context: vscode.ExtensionContext){
    // Register light commands!
    context.subscriptions.push(commands.docs);
}

// Activate the extension!
export function activate(context: vscode.ExtensionContext) {
    // Do lightweight activation first
    lightweightActivation(context);
    // Do more heavy stuff here
    // (Like transpiler communication!)
    // ...
}
