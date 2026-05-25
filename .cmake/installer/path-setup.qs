function Component() {
    // Default constructor
}

Component.prototype.createOperations = function() {
    component.createOperations();

    // Define the path to bin directory
    var binPath = "@TargetDir@/bin";

    // Inject the PATH variable
    // On Windows, this cleanly edits the Registry. 
    // On Linux/Mac, it attempts to edit ~/.profile or /etc/paths.
    component.addOperation("EnvironmentVariable", "PATH", binPath, "true");
}
