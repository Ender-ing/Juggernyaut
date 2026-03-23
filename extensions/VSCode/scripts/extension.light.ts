/**
 *
 * Start the extension!
 * (for web browsers)
 *
**/

// Imports
import * as vscode from "vscode";
import * as lightCommands from "./commands.light";

// Activate the extension!
export async function activate(context: vscode.ExtensionContext) {
    // Add basic commands
    lightCommands.register(context)
}
