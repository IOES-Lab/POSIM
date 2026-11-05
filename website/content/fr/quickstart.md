# Première simulation

Lancez un REXROV dans l'océan, recevez ses mesures, puis arrêtez la simulation. Conservez le robot et le monde proposés pour ce premier essai.

## Préparer les terminaux

Terminez l'[installation](requirements.md), puis ouvrez [deux terminaux avec ROS et POSIM chargés](terminals.md). Le terminal A exécute le simulateur ; B consulte les données. Avec Docker, Gazebo s'affiche dans le bureau Linux, WSLg ou RDP configuré précédemment.

## 1. Lancer le robot et le monde

Dans le **terminal A** :

{{CODE:quickstart:0}}

REXROV apparaît cinq mètres sous z = 0 et ses ponts ROS démarrent. Cette commande lance aussi le monde : utilisez une seule simulation et gardez A ouvert.

## 2. Lire la position

Dans le **terminal B** :

{{CODE:quickstart:1}}

Les mesures doivent arriver régulièrement. Une position presque constante peut être normale pendant que le robot se stabilise. Appuyez sur **Ctrl+C dans B** pour arrêter l'affichage.

## 3. Vérifier le temps simulé

{{CODE:quickstart:2}}

L'horloge doit avancer. Arrêtez son affichage avec Ctrl+C. Si aucun message n'arrive ou si le temps reste figé, consultez le [dépannage](troubleshooting.md).

## 4. Terminer la session

Appuyez sur **Ctrl+C dans A** et attendez la fin des processus. Fermer un affichage de topic n'arrête pas le simulateur. Arrêtez cette simulation avant d'en lancer une autre.

## Poursuivre

- [Afficher et enregistrer les capteurs](visualization.md) : voir les images et sauvegarder les observations.
- [Commander avec Python](python-control.md) : lire l'état et actionner un propulseur.
- [ROV et BlueROV2](rovs.md) : ArduSub, clavier et joystick.
- [Bibliothèque de mondes](worlds.md) : choisir un autre environnement.
