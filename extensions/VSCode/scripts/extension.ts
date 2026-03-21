/**
 *
 * Start the extension!
 *
**/

// Imports
import * as vscode from "vscode";
import * as commands from "./commands";
import * as server from "./server"

// Lightweight extension activation! (for web browsers!)
export function lightweightActivation (context: vscode.ExtensionContext){
    // Register light commands!
    context.subscriptions.push(commands.docs);
}

// Activate the extension!
export async function activate(context: vscode.ExtensionContext) {
    // Do lightweight activation first
    lightweightActivation(context);
    // Start the language server
    await server.start();
}

export function deactivate(): Thenable<void> | undefined {
    if (!server.client) {
        return undefined;
    }
    return server.client.stop();
}
