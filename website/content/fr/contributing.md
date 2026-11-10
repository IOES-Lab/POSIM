# Contribuer à POSIM

Les contributions au code et à la documentation sont les bienvenues. Commencez par une modification ciblée et reproductible, en gardant modèles, contrôleurs et guides cohérents.

## Travailler dans une branche

{{CODE:contributing:0}}

Pour modifier la simulation, utilisez l'[environnement d'installation](requirements.md) adapté. N'ajoutez pas au commit les répertoires de compilation ou d'installation générés, ni les identifiants locaux.

## Vérifier le comportement modifié

Choisissez un exemple minimal pertinent. Vérifiez les ressources, l'apparition du modèle, l'avancement du temps, les données reçues, le contrôleur si nécessaire et l'arrêt normal. Notez la révision, le moteur de rendu, l'architecture et la commande.

Les changements de capteurs ou de dynamique demandent une comparaison numérique adaptée. Une capture montre l'apparence, pas la justesse du modèle physique. Conservez les attributions d'origine lorsque vous modifiez du code hérité.

## Mettre à jour les huit langues

Le site propose l'anglais, le coréen, l'espagnol, le français, l'allemand, l'italien, le japonais et le chinois. L'anglais utilise `/` ; les autres langues utilisent `/ko/`, `/es/`, `/fr/`, `/de/`, `/it/`, `/ja/` et `/zh/`.

Mettez à jour les fichiers correspondants dans `website/content/<langue>`. Conservez les mêmes commandes, unités, prérequis et limites, en rédigeant naturellement avec les termes usuels de la robotique. Les marqueurs partagés `CODE` et `MEDIA` réutilisent les exemples exécutables et les illustrations. Toutes les pages sont requises dans chaque langue ; un texte absent n'est pas remplacé par de l'anglais.

Pour une fonction publique :

- Mettez à jour le [catalogue des fonctions](features.md) et le guide d'utilisation avec le code.
- Précisez installation et activation, valeurs par défaut, unités, entrées et sorties, portée des réglages, exemple observable et limites du modèle. Incluez les effets facultatifs et les adaptateurs.
- Inscrivez les nouvelles pages dans `website/scripts/pages.mjs` et leurs titres dans `website/i18n/<langue>.json`. Mettez aussi à jour les README de chaque langue.

L'ordre des pages est commun ; les catalogues de mondes et d'objets proviennent des sources. Décrivez le produit actuel. Gardez les notes internes, plans et rapports de validation hors du dépôt ; utilisez les issues et pull requests pour discuter des changements et de leurs résultats.

## Vérifier le site

Depuis `website`, avec Node.js 24 et pnpm 10.11.0 :

{{CODE:contributing:1}}

Ouvrez `http://127.0.0.1:4174`. Vérifiez les liens, la copie des commandes, la recherche, le changement de langue et la navigation mobile. La publication et la maintenance sont décrites dans [website/README.md](https://github.com/IOES-Lab/POSIM/blob/main/website/README.md).

## Ouvrir une pull request

Décrivez le problème, le comportement obtenu et les vérifications effectuées. Pour une modification de simulation, fournissez les commandes et journaux pertinents ; pour la mise en page, des vues sur ordinateur et mobile. Distinguez les points vérifiés de ceux qui restent à tester.

Utilisez les [issues GitHub](https://github.com/IOES-Lab/POSIM/issues) pour les défauts reproductibles et les [pull requests](https://github.com/IOES-Lab/POSIM/pulls) pour la revue. IOES-Lab maintient le projet à la [Korea Maritime & Ocean University](https://lab.wschoi.com).
