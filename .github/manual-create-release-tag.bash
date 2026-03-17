# Alpha
# (Used to release very early and untested binaries)
git tag -a "v0.0.0-alpha.000" -m "Alpha #000 v0.0.0"

# Beta
# (Used to release experimental binaries)
git tag -a "v0.0.0-beta.00" -m "Beta #00 v0.0.0"

# Prerelease
# (Used to release polished binaries)
git tag -a "v0.0.0-prerelease.00" -m "Prerelease #00 v0.0.0"

# Release
# (Used to release the final release)
git tag -a "v0.0.0-release" -m " Release v0.0.0"


# Push the tag into the main repository to create a release draft!
git push origin "v0.0.0-?"

# Use this command to remove a local tag that was created before the latest commit!
git tag -d "v0.0.0-?"
