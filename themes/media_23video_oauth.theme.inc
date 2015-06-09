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

/**
 * Implements TEME_preprocess().
 *
 * For video preview.
 */
function media_23video_oauth_preprocess_media_23video_preview(&$variables) {
  foreach ($variables['options'] as $key => $val) {
    $variables[$key] = $val;
  }
}

function media_23video_oauth_preprocess_media_23video_form_table(&$variables) {
  $variables['roles'] = array();
  if (!empty($variables['form']['roles'])) {
    foreach ($variables['form']['roles'] as $id => $role) {
      if (substr($id, 0, 1) != '#') {
        $variables['roles'][$id] = $role;
      }
    }
  }
  $variables['before'] = array();
  if (!empty($variables['form']['before'])) {
    foreach ($variables['form']['before'] as $id => $item) {
      if (substr($id, 0, 1) != '#') {
        $variables['before'][$id] = $item;
      }
    }
  }
  $variables['after'] = array();
  if (!empty($variables['form']['after'])) {
    foreach ($variables['form']['after'] as $id => $item) {
      if (substr($id, 0, 1) != '#') {
        $variables['after'][$id] = $item;
      }
    }
  }
  $variables['rest'] = array();
  foreach ($variables['form'] as $id => $item) {
    if (!in_array($id, array('roles', 'before', 'after', 'submit')) && substr($id, 0, 1) != '#') {
      $variables['rest'][$id] = $item;
    }
  }
}
