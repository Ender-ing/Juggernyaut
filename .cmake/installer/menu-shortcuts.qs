function Component() {
    // Default constructor
}

Component.prototype.createOperations = function() {
    // Call default implementation first
    component.createOperations();

    if (systemInfo.productType === "windows") {
        // Create the Uninstall/Modify shortcut
        component.addOperation(
            "CreateShortcut",
            "@TargetDir@\\maintenancetool.exe",
            "@StartMenuDir@\\Juggernyaut Toolchain Uninstaller.lnk",
            "description=Modify or Uninstall the Juggernyaut Toolchain"
        );
        // NOT WORKING:
        /*component.addOperation(
            "CreateShortcut",
            "@TargetDir@\\jug-docs.html",
            "@StartMenuDir@\\Juggernyaut Documentation.lnk",
            "description=Browse the Online Juggernyaut Documentation"
        );*/
    }
}
