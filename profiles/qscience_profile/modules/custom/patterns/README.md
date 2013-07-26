# Patterns, Drupal 7.x module

Complex websites and web applications can be created by combining
configurations of Modules, Fields, Content Types, Menus, Blocks,
Categories, Roles / Permissions, etc... This site setup and
configuration process is a very time consuming and repetitive
bottleneck.

The Patterns module is built to bypass this bottleneck by managing and
automating site configuration. Site configuration is stored in YAML, XML, or
PHP files called Patterns. These files have a structure which is
easy to read, modify, manage, & share and can be executed manually or as
a part of an automated web site deployment.


# From 7.x-1.x to 7.x-2.x

This is a major release and you might find that the syntax of some of the Patterns might have changed/being extended. Backward compatibility with Patterns created with the syntax of 7.x-1.x branch is not supported.

== Syntactic and semantic validation ==

The new version of Patterns allows the components to implement two separated layers of syntactic and semantic validation, providing a very valuable feedback to the user that is going to run a pattern in case it is necessary to solve any possible conflicts.
Improvement of the export functions and components extension

== Improvement of the export functions and components extension ==

The new version of Patterns provides a set of enhancements to export automatically the configuration of a site. All the main components possess now automatic export capabilities, and depending on the type of component new options to export it are offered. Two different use cases are distinguised:

    Export the configuration as a pattern consisting of a set of CREATE actions which allows users the creation of patterns for “fresh installation” of certain feature,
    Export the configuration as a pattern consisting of a set of MODIFY actions
    which allows users the creation of patterns to override the current settings (e.g.,
    to update the features developed in a testing site to a production site). The patterns generated ensure idempotency in the operations.

== Patterns Server ==

The new version of Patterns provides the changes to offer compatibility with the new Patterns Server module (to be released soon), whose aim is to act as a hub for sharing Patterns files among Drupal users.

# Documentation

A fairly complete and up to date documentation is available here:

[drupal.org](http://drupal.org/node/1464118).
