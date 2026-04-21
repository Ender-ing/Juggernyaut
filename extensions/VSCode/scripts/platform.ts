/**
 *
 * Gather information about the current platform!
 *
**/

// Imports
import { window, workspace, env, Uri, UIKind } from 'vscode';

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

export async function getCommand(base:string, name:string): Promise<{cmd: string, args: string[]}> {
    const platform = await getPlatform();
    if (platform == Platform.Windows) {
        const cmd: string = Uri.joinPath(Uri.file(base), `${name}.exe`).fsPath;
        return {
            cmd: cmd,
            args: []
        };
    } else if (platform == Platform.Linux || platform == Platform.Mac) {
        const config = workspace.getConfiguration('juggernyaut');
        const shouldUseValgrind = config.get<boolean>('debug.valgrind');

        const cmd: string = Uri.joinPath(Uri.file(base), name).fsPath;
        if (shouldUseValgrind) {
            return {
                cmd: "valgrind",
                args: [
                    "--leak-check=full",
                    "--show-leak-kinds=all",
                    "--track-origins=yes",
                    `--log-file=${base}/${name}.valgrind.log`,
                    cmd
                ]
            };
        } else {
            return {
                cmd: cmd,
                args: []
            };
        }
    } else {
        window.showInformationMessage("Your platform isn't supported by Juggernyaut's language server!");
        return {
            cmd: "",
            args: []
        };
    }
}
