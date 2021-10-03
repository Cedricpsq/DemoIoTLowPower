# DemoIoTLowPower

L'objectif de se projet est d'utiliser les spécifisités de l'esp32 afin de faire de la basse conssomation.
Pour se faire nous avons mise en place une machine d'état afin que le module passe en deep sleep lorsqu'il n'est pas utilisé.
Le système permet de regarder

# Installation

Dans l'objectif de récupérer la température actuelle d'une ville, nous utilisons l'API du site weatherstack.
Pour utiliser l'api, il faut récupérer la clé liée à son compte.

Photo

2 - Paramétrer dans le code.

Afin de faire fonctionner la démo, il faut configurer certaines variables dans le code.
Premièrement pour le wifi, afin que l'esp32 se connecte au Wifi, il faut lui indiquer le nom et le mot de passe.
Deuxièmement, il faut placer sa clé personnelle du site weatherstack.com dans la constante "privateKey"

Photo

3 - Branchement

Si vous avez une esp32 vous pouvez suivre le schéma ci-dessous.

Photo

# Fonctionnement

Le système va récupérer une première fois les données de l'API durant le démarrage.
Par la suite, chaque minute, le système va mettre à jour ses données, le rythme peut être modifié dans le code avec la constante TIME_TO_SLEEP.
Lorsque les données sont à jour et que le système est en veille, l'appui sur le bouton-poussoir va allumer le module et afficher la dernière température enregistrer sur l'écran.
Après 6 secondes, le système revient en veille.
