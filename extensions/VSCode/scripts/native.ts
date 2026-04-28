/**
 *
 * Handle native actions
 *
**/

// Imports
import { Uri } from 'vscode';
import * as which from 'which';
import { Section, info } from './channel';

export async function findGlobalToolchainPath(): Promise<string|undefined> {
    // 'nothrow: true' prevents it from throwing a hard error if not found
    const resolvedPath = await which.sync('jug', { nothrow: true });

    if (resolvedPath) {
        info(Section.NATIVE, `Found global command at: ${resolvedPath}`);
        const toolchainPath: string = Uri.joinPath(Uri.file(resolvedPath), '../..').fsPath;
        info(Section.NATIVE, `Detected global toolchain path: ${toolchainPath}`);
        return toolchainPath;
    }
    return undefined;
}