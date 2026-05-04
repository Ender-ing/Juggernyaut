/**
 *
 * Manage toolchain info!
 *
**/

import { window, commands, workspace } from 'vscode';
import { findGlobalToolchainPath } from './native';

export async function getPath(): Promise<string> {
    // Get the current config value
    const config = workspace.getConfiguration('juggernyaut');
    const toolchainPath = config.get<string>('toolchainPath');

    // Check the value
    if (!toolchainPath || toolchainPath.trim() === '') {
        const globalToolchainPath: string|undefined = await findGlobalToolchainPath();
        if (!globalToolchainPath || globalToolchainPath.trim() == '') {
            // Warn the user about the missing path value!
            const userChoice = await window.showWarningMessage(
                'The Juggernyaut toolchain path is not set, and no global alternative has been found! Some features are disabled.',
                'Open Settings'
            );

            // Route to the setting
            if (userChoice === 'Open Settings') {
                await commands.executeCommand(
                    'workbench.action.openSettings', 
                    'juggernyaut.toolchainPath'
                );
            }

            // Halt the process!
            return "";
        }

        return globalToolchainPath;
    }
    // Return the path!
    return toolchainPath;
}
