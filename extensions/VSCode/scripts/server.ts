import { Uri } from 'vscode';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind,
    Executable} from 'vscode-languageclient/node';
import * as sdk from "./sdk"
import { getCommand } from './platform';

export let client: LanguageClient;

export async function start() {
    // Define the path to the executable!
    const sdkPath: string = await sdk.getPath();
    if(sdkPath == "") {
        return;
    }
    const binPath: string = Uri.joinPath(Uri.file(sdkPath), "bin").fsPath;
    const command = await getCommand(binPath, "jug-lsp");

    // 2. Define how the server is launched
    const executable: Executable = {
        command: command,
        transport: TransportKind.stdio,
        args: [] // Add any command-line arguments your C++ server expects here
    };

    const serverOptions: ServerOptions = {
        run: executable,
        debug: executable // You can pass different args here if you have a debug mode
    };

    // 3. Define the client options
    const clientOptions: LanguageClientOptions = {
        // Register the server for your custom language. 
        // 'yourLanguageId' must match the language id defined in your package.json
        documentSelector: [{ scheme: 'file', language: 'juggernyaut' }]
    };

    // Create the language client and start the client.
    client = new LanguageClient(
        'juggernyaut',
        'Juggernyaut Language Server',
        serverOptions,
        clientOptions
    );
    client.start();
}
