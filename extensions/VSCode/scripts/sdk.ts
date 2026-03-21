/**
 *
 * Manage SDK info!
 *
**/

import { window, commands, workspace } from 'vscode';

export async function getPath(): Promise<string> {
    // Get the current config value
    const config = workspace.getConfiguration('juggernyaut');
    const sdkPath = config.get<string>('sdkPath');

    // Check the value
    if (!sdkPath || sdkPath.trim() === '') {
        // Warn the user about the missing path value!
        const userChoice = await window.showWarningMessage(
            'The Juggernyaut SDK path is not set. Some features are disabled.',
            'Open Settings'
        );

        // Route to the setting
        if (userChoice === 'Open Settings') {
            await commands.executeCommand(
                'workbench.action.openSettings', 
                'juggernyaut.sdkPath'
            );
        }

        // Halt the process!
        return "";
    }
    // Return the path!
    return sdkPath;
}
