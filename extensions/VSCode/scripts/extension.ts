/**
 *
 * Start the extension!
 *
**/

// Imports
import * as vscode from "vscode";
import * as lightCommands from "./commands.light";
import * as commands from "./commands";
import * as server from "./server"

// Activate the extension!
export async function activate(context: vscode.ExtensionContext) {
    // Register commands!
    lightCommands.register(context);
    commands.register(context)
    // Start the language server
    server.activate(context);
}

export async function deactivate(): Promise<Thenable<void>> {
    if (server.client == undefined) {
        return undefined;
    }
    return await server.deactivate();
}
