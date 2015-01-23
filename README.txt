= Media 23Video OAuth
Media 23Video OAuth integrates with the Media module to upload videos to and show videos from 23Video when using access restrictions on videos. The module uses OAuth authentication via 23Video's own OAuth implementation (https://github.com/23/23video-for-php).

Users can upload files to the 23Video server without having their own login to the 23Video account. Thumbnails are downloaded dynamically.

This module is partially inspired from Media 23Video and spheresh's sandbox module Media 23Video auth.

Similar projects:
Media 23Video
Media 23Video auth sandbox module

# OAuth library
The module uses 23Video's library 23video-for-php https://github.com/23/23video-for-php.git
It's much smaller than the PEAR HTTP/OAuth library and doesn't have many dependencies (requires curl).
