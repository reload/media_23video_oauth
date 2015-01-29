<?php

/**
 * @file
 * Theme and preprocess functions for Media 23Video Oauth.
 */

/**
 * Implements THEME_preprocess().
 *
 * For error messages.
 */
function media_23video_oauth_preprocess_media_23video_error(&$variables) {
  foreach ($variables['options'] as $key => $val) {
    $variables[$key] = $val;
  }
}

/**
 * Implements THEME_preprocess().
 *
 * For video.
 */
function media_23video_oauth_preprocess_media_23video_video(&$variables) {
  foreach ($variables['options'] as $key => $val) {
    $variables[$key] = $val;
  }
  $variables['width'] = 640;
  $variables['height'] = 480;
}

function media_23video_oauth_preprocess_media_23video_preview(&$variables) {
  foreach ($variables['options'] as $key => $val) {
    $variables[$key] = $val;
  }
}
