D2D
===

## Introduction ##
D2D ("Drupal-to-Drupal") is a module to built-up a network among Drupal instances using cryptography and XML-RPC.

An instance holds a public/private key pair allowing messages to be encrypted and/or to be signed. The concept of friendship among Drupal instances is introduced. Instances can send friend requests, accept friendship requests and also terminate friendship.

An instance is identified using a globally unique D2D-id. The aim of this id is to solve the naming problem for D2D instances, i.e. you can uniquely refer to a certain instance by this D2D-id, compare the public keys you have stored for an instance with the ones your friends store etc..

Using public key cryptography, an instance can implement secure XML-RPC methods that can be called by friend instances. Friends are organized in groups to allow privileged access to particular methods.

### What D2D provides ###
* built-up a network of Drupal instances
* add Drupal instances as friends
* organize friends in groups
* easily implement remote functions and give privileged access to groups of friends

Since D2D is under development and mainly should be a proof of concept, there are many things / features, that are not provided, some features being possibly implemented in the future.

### What D2D does not provide (yet) ###
* the database is not locked before accessing, in particular concurrent access might corrupt data in the database
* D2D might be vulnerable to several remote attacks such as DoS-attacks
* ...

## Installation ##

### Requirements ###
* Drupal 7
* PHP 5.3 or higher
* PHP compiled with OpenSSL support
* MySQL database (most queries have already been rewritten using Drupal's database API)

### The installation ###

After installing D2D a menu entry called D2D appears in the admin's menu. The admin is asked to provide the name of this installation (a short name describing the instance), a D2D-identifier and the address under which this D2D instance is reachable. Note that the D2D-id can only be changed by reinstalling the module. It is recommended that one choses the randomly generated D2D-id and only enters the D2D-id manually in case one wants to reuse a previously used D2D-id (e.g. if one wants to reinstall D2D). Name and address can be changed at a later point. Further description on the required information is provided with the configuration form. Finally, there is a checkbox allowing to automatically choose a random public / private key pair and to set the state of this instance to online. If this option is chosen, the keys are automatically generated and the state of the instance is set to online which means that the instance is ready for communication with other instances. If one does not want to choose a public / private key pair yet, one can do so later using the _Settings_-tab.
After completing this process, the menu is rebuilt and one can access D2D using the D2D tab in the admins menu. All functionality provided by D2D can be accessed using this tab.

Note that for some functionality (in particular for sending a friend request / replying to a friendship request to another instance that is not reachable at the moment) D2D requires Drupal's cron to be setup correctly and called periodically, e.g. every 15 minutes. See <http://drupal.org/cron> and <http://drupal.org/node/23714> for details on how to setup cron.

## License ##

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
