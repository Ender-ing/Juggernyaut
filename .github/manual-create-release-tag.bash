# Alpha
# (Used to release very early and untested binaries)
git tag -a "v0.0.1-alpha.000" -m "Alpha #000 v0.0.1"

# Beta
# (Used to release experimental binaries)
git tag -a "v0.0.1-beta.00" -m "Beta #00 v0.0.1"

# Prerelease
# (Used to release polished binaries)
git tag -a "v0.0.1-prerelease.00" -m "Prerelease #00 v0.0.1"

# Release
# (Used to release the final release)
git tag -a "v0.0.1-release" -m " Release v0.0.1"


# Push the tag into the main repository to create a release draft!
git push origin "v0.0.1-?"

# Use this command to remove a local tag that was created before the latest commit!
git tag -d "v0.0.1-?"
