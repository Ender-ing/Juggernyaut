/**
 *
 * Manage SDK info!
 *
**/

import { window, commands, workspace } from 'vscode';
import { findGlobalSDKPath } from './native';

export async function getPath(): Promise<string> {
    // Get the current config value
    const config = workspace.getConfiguration('juggernyaut');
    const sdkPath = config.get<string>('sdkPath');

    // Check the value
    if (!sdkPath || sdkPath.trim() === '') {
        const globalSDKPath: string|undefined = await findGlobalSDKPath();
        if (!globalSDKPath || globalSDKPath.trim() == '') {
            // Warn the user about the missing path value!
            const userChoice = await window.showWarningMessage(
                'The Juggernyaut SDK path is not set, and no global alternative has been found! Some features are disabled.',
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

        return globalSDKPath;
    }
    // Return the path!
    return sdkPath;
}
