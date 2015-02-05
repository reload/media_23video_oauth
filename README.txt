INTRODUCTION
------------
Media 23Video OAuth integrates with the Media module to upload videos to and
show videos from 23Video when using access restrictions on videos. The module
uses OAuth authentication via 23Video's own OAuth implementation
(https://github.com/23/23video-for-php).

Users can upload files to the 23Video server without having their own login to
the 23Video account. Thumbnails are downloaded dynamically.

This module is partially inspired from Media 23Video and spheresh's sandbox
module Media 23Video auth.

 * For a full description of the module, visit the project page:
   https://www.drupal.org/sandbox/bartvig/2412809

 * To submit bug reports and feature suggestions, or to track changes:
   https://www.drupal.org/project/issues/2412809

REQUIREMENTS
------------
This module requires the following modules:
 * Media (http://drupal.org/project/media)

This module also requires curl to be installed on the system.

SIMILAR PROJECTS
----------------
Media 23Video (https://www.drupal.org/project/media_23video)
Media 23Video provides support for publicly available videos on 23Video. This
module is different in that it provides OAuth support for access to uploading
and viewing videos on a 23Video account.

Media 23Video auth sandbox module
(https://www.drupal.org/sandbox/spheresh/2111835) Media 23Video auth provides
basic support for authenticating via OAuth, but doesn't yet support uploading or
viewing videos. The module also uses a different OAuth library.

BUNDLES
-------
The module bundles 23Video's library 23video-for-php
https://github.com/23/23video-for-php.git
It's much smaller than other OAuth libraries and doesn't have many dependencies
(requires curl).

INSTALLATION
------------
Start by downloading the Libraries module.

Next download 23 Video for PHP under sites/all/libraries. You can do this by
standing in the correct folder and performing the following command:
  git clone https://github.com/23/23video-for-php.git

Next, install as you would normally install a contributed Drupal module. See:
  https://drupal.org/documentation/install/modules-themes/modules-7
  for further information.

CONFIGURATION
-------------
 * Configure user permissions in Administration » People » Permissions:
   - See videos with restricted access
     Users that should be allowed to see the videos uploaded to the 23Video
     account should have this permission.
   - Administer 23Video
     Admins with access to the 23Video account should have this permission.
 * Configure the 23Video account's access and api keys in Configuration » Media
 * » Media 23Video Settings.
   - This is required to connect the Drupal site with the 23Video account.
   - To set this up on your 23Video account
     - go to <account name>.reload.com/manage/api and add privileged access
     - Copy consumer key, consumer secret, access token, and access token secret
       to the module settings
