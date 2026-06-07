# Pyramid Dungeon - VS 2026 Edition

Jeu 3D solo en C++ avec raylib : dungeon crawler / roguelite dans une pyramide souterraine.

Cette version est adaptée pour Visual Studio Community 2026 Insider / Build Tools 2026.
Elle évite volontairement `CXX_STANDARD 18`, parce que `C++18` n'existe pas. Le projet force `/std:c++17` directement avec MSVC, histoire que CMake arrête d'inventer des standards comme un mage ivre.

## Contenu jouable

- Hub sécurisé : Sanctuaire de l'Entrée.
- Choix de classe : mage élémentaire, nécromancien, combattant, moine.
- Marchand d'armes.
- Marchand de consommables.
- Vendeur de loots avec vente automatique des objets communs.
- Atelier de transformation de ressources.
- Autel d'améliorations permanentes.
- Coffre personnel sauvegardé.
- Donjon 3D avec 3 étages.
- Salles de combat, pièges, coffre, repos, maudites, mini-boss, transition et boss.
- Spawns de mobs par salle.
- Loots par type d'ennemi.
- Pièges : pics, feu, sable mouvant, sol fragile.
- Pierre de Retour avec incantation de 3 secondes.
- Cristal de Retour Stable instantané.
- Boss final : Gardien de la Pyramide, avec phase 2.
- Sauvegarde locale dans `save.txt`.

## Contrôles

- `ZQSD` / `WASD` : déplacement.
- Souris : caméra.
- `Clic gauche` : attaque principale.
- `Clic droit` : compétence secondaire.
- `Espace` : esquive / dash.
- `E` : interagir.
- `A` : utiliser potion de soin.
- `R` : utiliser retour au spawn dans le donjon.
- `TAB` : inventaire.
- `Échap` : pause / retour menu.
- `F1` : libérer ou capturer la souris.
- Au hub : `1`, `2`, `3` pour entrer à un étage débloqué.

## Prérequis Windows

- Windows 10 ou Windows 11 avec `winget` disponible.
- Une connexion Internet pour la première compilation.

Le compilateur installe automatiquement les outils manquants :

- CMake ;
- Git, nécessaire pour télécharger raylib ;
- Visual Studio Build Tools 2026 ;
- le compilateur MSVC x64 et le Windows SDK via le workload C++.

L'installation Visual Studio peut demander une confirmation administrateur et prendre plusieurs minutes.

## Build recommandé avec Visual Studio 2026

Double-clique sur :

```text
build_windows_vs2026.bat
```

Tu peux également utiliser `AAA_build_windows_vs2026.bat`. Les deux scripts lancent
le même installateur-compilateur. Il n'est pas nécessaire d'ouvrir un terminal
développeur Visual Studio.

Ou lance à la main depuis le dossier du projet :

```powershell
cmake --preset vs2026-release
cmake --build --preset build-vs2026-release
```

L'exécutable sort ici :

```text
build-vs2026\Release\PyramidDungeon.exe
```

## Build alternatif avec Ninja + MSVC

À utiliser si ton CMake ne reconnaît pas encore le générateur Visual Studio 2026.

Ouvre :

```text
Developer PowerShell for VS 2026
```

Puis lance :

```text
build_windows_ninja_msvc.bat
```

Ou à la main :

```powershell
cmake --preset ninja-msvc-release
cmake --build --preset build-ninja-msvc-release
```

L'exécutable sort ici :

```text
build-ninja\PyramidDungeon.exe
```

## Nettoyer le build

```text
clean_build.bat
```

## Lancer l'exe

Après compilation :

```text
run_release.bat
```

## Notes importantes

Le projet utilise raylib via `FetchContent`. Au premier build, CMake télécharge raylib depuis GitHub. Après ça, il le garde dans le dossier de build.

Les graphismes utilisent des formes 3D simples, mais les systèmes sont séparés pour pouvoir remplacer les cubes/sphères par des modèles `.glb` plus tard.

Le fichier `save.txt` est créé à côté de l'exécutable quand le jeu sauvegarde.

## Architecture

```text
/PyramidDungeon_VS2026
  /assets
    /models
    /textures
    /sounds
  /include
    Boss.h
    Common.h
    CraftingStation.h
    Dungeon.h
    Enemy.h
    Game.h
    Inventory.h
    ItemDatabase.h
    Loot.h
    Player.h
    Projectile.h
    Room.h
    SaveSystem.h
    Shop.h
    Trap.h
    Types.h
    UI.h
    UpgradeStation.h
  /src
    Boss.cpp
    CraftingStation.cpp
    Dungeon.cpp
    Enemy.cpp
    Game.cpp
    Inventory.cpp
    ItemDatabase.cpp
    Loot.cpp
    Player.cpp
    Projectile.cpp
    Room.cpp
    SaveSystem.cpp
    Shop.cpp
    Trap.cpp
    UI.cpp
    UpgradeStation.cpp
    main.cpp
  CMakeLists.txt
  CMakePresets.json
  build_windows_vs2026.bat
  build_windows_ninja_msvc.bat
  build_windows_msvc.bat
  clean_build.bat
  run_release.bat
```

## Correction VS 2026 / CXX18

Si tu avais cette erreur :

```text
Target "PyramidDungeon" requires the language dialect "CXX18"
```

elle vient d'un mauvais standard CMake. Cette version corrige ça avec :

```cmake
if (MSVC)
    target_compile_options(PyramidDungeon PRIVATE /std:c++17 /W3 /permissive- /Zc:__cplusplus)
else()
    target_compile_features(PyramidDungeon PRIVATE cxx_std_17)
endif()
```

Donc pas de `CXX_STANDARD 18`, pas de `CXX18`, pas de rituel satanique dans le build system.
