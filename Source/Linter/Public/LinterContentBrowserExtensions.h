// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

#pragma once

class FLinterModule;

// Integrate Linter actions into the Content Browser
class FLinterContentBrowserExtensions {
public:
    static void InstallHooks();
    static void RemoveHooks();
};
