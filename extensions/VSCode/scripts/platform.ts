/**
 *
 * Gather information about the current platform!
 *
**/

// Imports
import { window, env, Uri, UIKind } from 'vscode';

export enum Platform {
    Web = 0,
    Windows = 1,
    Linux = 2,
    Mac = 3
}

export function getPlatform(): Platform {
    if (env.uiKind == UIKind.Web) {
        return Platform.Web
    } else {
        const osPlatform = process.platform; // Returns 'win32', 'darwin', or 'linux'
        if (osPlatform === 'win32') {
            return Platform.Windows;
        } else if (osPlatform === 'darwin') {
            return Platform.Mac;
        } else {
            return Platform.Linux;
        }
    }
}

export async function getCommand(base:string, name:string): Promise<string> {
    const platform = await getPlatform();
        if (platform == Platform.Windows) {
            return Uri.joinPath(Uri.file(base), `${name}.exe`).fsPath;
        } else if (platform == Platform.Linux || platform == Platform.Mac) {
            return Uri.joinPath(Uri.file(base), name).fsPath;
        } else {
            window.showInformationMessage("Your platform isn't supported by Juggernyaut's language server!");
            return "";
        }
    
}
