/**
 * 
 * Take care of extension commands!
 * 
**/

// Imports
import { commands, env, Uri, ExtensionContext } from "vscode";
import { outputChannel } from "./channel";

// Constants
const DOCS_URL = 'https://ender.ing/docs/juggernyaut/';

// Light commands
const docs = commands.registerCommand('juggernyaut.docs', () => {
    const url = Uri.parse(DOCS_URL);
    env.openExternal(url);
});
const internal = commands.registerCommand('juggernyaut.internal', async () => {
    outputChannel.show(true);
});

export function register(context: ExtensionContext) {
    context.subscriptions.push(docs, internal);
}
