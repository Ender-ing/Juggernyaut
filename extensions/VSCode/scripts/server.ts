/**
 *
 * Manage the Language Server!
 *
**/

import { workspace, Uri, ExtensionContext  } from 'vscode';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind,
    Executable,
    Message,
    ErrorAction,
    CloseAction} from 'vscode-languageclient/node';
import * as toolchain from "./toolchain"
import { getCommand } from './platform';
import { info, error, Section } from './channel';

export let client: LanguageClient | undefined = undefined;
let haultCycle: boolean = false;
let spawnCount: number = 0;

async function start() {
    // Define the path to the executable!
    const toolchainPath: string = await toolchain.getPath();
    if(toolchainPath == "") {
        return;
    }
    const binPath: string = Uri.joinPath(Uri.file(toolchainPath), "bin").fsPath;
    const command = await getCommand(binPath, "jug-lsp");

    // 2. Define how the server is launched
    const executable: Executable = {
        command: command.cmd,
        transport: TransportKind.stdio,
        args: command.args
    };

    const serverOptions: ServerOptions = {
        run: executable,
        debug: executable // You can pass different args here if you have a debug mode
    };

    // 3. Define the client options
    const clientOptions: LanguageClientOptions = {
        // Register the server for your custom language. 
        // 'yourLanguageId' must match the language id defined in your package.json
        documentSelector: [{ scheme: 'file', language: 'juggernyaut' }],
        
        // Override the default restart behavior
        errorHandler: {
            error: (err: Error, message: Message | undefined, count: number | undefined) => {
                
                error(Section.SERVER, "An Error occurred!", err);
                // Triggered when the server encounters an error.
                // Tell the client to shut down the server instead of continuing.
                return { action: ErrorAction.Shutdown };
            },
            closed: () => {

                error(Section.SERVER, "The server connection has been closed!");
                // Triggered when the server connection is closed/crashes.
                // Tell the client to absolutely not restart it.
                return { action: CloseAction.DoNotRestart };
            }
        }
    };

    // Create the language client and start the client.
    client = new LanguageClient(
        'juggernyaut',
        `Juggernyaut Language Server (${++spawnCount})`,
        serverOptions,
        clientOptions
    );
    client.start();
}

// Restrict language server runtime
let isChangingServerState = false;
export async function manageServerLifecycle() {
    if (isChangingServerState || haultCycle) {
        return;
    }
    const hasActiveFiles = workspace.textDocuments.some(
        (doc) => doc.languageId === 'juggernyaut' 
    );

    // Start/stop the server
    if (hasActiveFiles && client == undefined) { // Start
        isChangingServerState = true;
        await start();
        isChangingServerState = false;
        info(Section.SERVER, 'Starting the Juggernyaut language server...');
    } else if (!hasActiveFiles && client != undefined) { // Stop
        isChangingServerState = true;
        await client.stop();
        client = undefined; // Clear the reference so it can be restarted
        isChangingServerState = false;
        info(Section.SERVER, 'Terminating the Juggernyaut language server...');
    }
}
export function enableServerCycle(state: boolean) {
    haultCycle = !state;
}

// Prevent excessive updates
let debounceTimeout: NodeJS.Timeout | undefined;
function debouncedLifecycleCheck() {
    if (debounceTimeout) {
        clearTimeout(debounceTimeout);
    }
    
    // Start a new timer
    debounceTimeout = setTimeout(() => {
        manageServerLifecycle();
    }, 150);
}

export function activate(context: ExtensionContext) {
    manageServerLifecycle();
    context.subscriptions.push(
        workspace.onDidChangeConfiguration(debouncedLifecycleCheck),
        workspace.onDidOpenTextDocument(debouncedLifecycleCheck),
        workspace.onDidCloseTextDocument(debouncedLifecycleCheck),
        workspace.onDidRenameFiles(debouncedLifecycleCheck),
        workspace.onDidCreateFiles(debouncedLifecycleCheck),
        workspace.onDidDeleteFiles(debouncedLifecycleCheck),
        workspace.onDidChangeWorkspaceFolders(debouncedLifecycleCheck)
    );
}

export async function deactivate() {
    info(Section.SERVER, "Deactivation Request received!")
    if (client == undefined) {
        info(Section.SERVER, "Server is already inactive!")
        return undefined;
    }
    if (isChangingServerState == true) {
        await new Promise(resolve => setTimeout(resolve, 150));
        return await deactivate();
    }
    isChangingServerState = true;
    await client.stop();
    client = undefined; // Clear the reference so it can be restarted
    isChangingServerState = false;
    info(Section.SERVER, "Server has been successfully deactivated!")
}
