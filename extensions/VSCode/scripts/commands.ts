/**
 * 
 * Take care of Omniarium extension commands!
 * 
**/

// Imports
import { window, commands, env, Uri, ExtensionContext } from "vscode";
import { deactivate, enableServerCycle, manageServerLifecycle } from "./server";
import { info, outputChannel, Section } from "./channel";

// Constants
const DOCS_URL = 'https://ender.ing/docs/juggernyaut/';

// Message test
const docs = commands.registerCommand('juggernyaut.docs', () => {
    const url = Uri.parse(DOCS_URL);
    env.openExternal(url);
});
const server = {
    stop: commands.registerCommand('juggernyaut.server.stop', async () => {
        info(Section.SERVER, "Deactivating the Juggernyaut language server...")
        enableServerCycle(false);
        await deactivate();
    }),
    resume: commands.registerCommand('juggernyaut.server.resume', async () => {
        info(Section.SERVER, "Resuming the Juggernyaut language server cycle...")
        enableServerCycle(true);
        await manageServerLifecycle();
    })
};

const internal = commands.registerCommand('juggernyaut.internal', async () => {
    outputChannel.show(true);
});


export function register(context: ExtensionContext) {
    context.subscriptions.push(docs, server.stop, server.resume, internal);
}