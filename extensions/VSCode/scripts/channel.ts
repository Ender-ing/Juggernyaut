/**
 *
 * Manage the output channel!
 *
**/

import { window, OutputChannel  } from 'vscode';

export const outputChannel: OutputChannel  = window.createOutputChannel('Juggernyaut (Internal)');

export enum Section {
    SERVER = "Language Server",
    NATIVE = "Native Configs"
}

export function info(section:Section, message: string) {
    // 2. Generate the timestamp to match your screenshot exactly.
    // toLocaleString() natively produces the "M/DD/YYYY, H:MM:SS AM/PM" format.
    const timestamp = new Date().toLocaleString('en-US');
    
    // 3. Write the formatted string to the output panel
    outputChannel.appendLine(`[${timestamp}] [${section}] ${message}`);
}

export function error(section:Section, message: string, error?: any) {
    const timestamp = new Date().toLocaleString('en-US');
    outputChannel.appendLine(`[${timestamp}] [${section}] ERROR: ${message}`);
    if (error) {
        outputChannel.appendLine(String(error));
    }
}
