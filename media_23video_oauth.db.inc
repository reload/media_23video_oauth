<?php

function media_23video_oauth_db_add($consumer_key, $consumer_secret, $oauth_token, $oauth_token_secret) {
  $query = db_select('media_23video_oauth', 'm');
  $query->fields('m');
  $query->condition('m.consumer_key', $consumer_key);
  $query->condition('m.consumer_secret', $consumer_secret);
  $res = $query->execute();

  // Don't create if it already exists.
  if ($res->rowCount() > 0) {
    return FALSE;
  }

  $query = db_insert('media_23video_oauth');
  $query->fields(array(
    'consumer_key' => $consumer_key,
    'consumer_secret' => $consumer_secret,
    'oauth_token' => $oauth_token,
    'oauth_token_secret' => $oauth_token_secret,
  ));
  $query->execute();
}

function media_23video_oauth_db_update($consumer_key, $consumer_secret, $oauth_token, $oauth_token_secret, $domain, $user_id, $user_name, $user_email, $site_id, $site_name) {
  $query = db_update('media_23video_oauth');
  $query->fields(array(
    'oauth_token' => $oauth_token,
    'oauth_token_secret' => $oauth_token_secret,
    'domain' => $domain,
    'user_id' => $user_id,
    'user_name' => $user_name,
    'user_email' => $user_email,
    'site_id' => $site_id,
    'site_name' => $site_name,
    'authenticated' => 1,
  ));
  $query->condition('consumer_key', $consumer_key);
  $query->condition('consumer_secret', $consumer_secret);
  $query->execute();
}

/**
 * Get application by consumer key/secret and optional oauth token and secret.
 *
 * @param string $consumer_key
 *   Consumer key
 *
 * @param string $consumer_secret
 *   Consumer secret
 *
 * @param string $oauth_token
 *   OAuth token
 *
 * @param string $oauth_token_secret
 *   OAuth token secret
 *
 * @return array
 *   Hash array
 */
function media_23video_oauth_db_get($consumer_key, $consumer_secret, $oauth_token = NULL, $oauth_token_secret = NULL) {
  $query = db_select('media_23video_oauth', 'm');
  $query->fields('m');
  $query->condition('m.consumer_key', $consumer_key);
  $query->condition('m.consumer_secret', $consumer_secret);
  if ($oauth_token) {
    $query->condition('m.oauth_token', $oauth_token);
  }
  if ($oauth_token_secret) {
    $query->condition('m.oauth_token_secret', $oauth_token_secret);
  }
  $res = $query->execute();

  if($res->rowCount() == 0) {
    return array();
  }

  $record = $res->fetchAssoc();
  return $record;
}

function media_23video_oauth_db_get_by_oauth_token($oauth_token) {
  if (is_null($oauth_token)) {
    return FALSE;
  }
  $query = db_select('media_23video_oauth', 'm');
  $query->fields('m');
  $query->condition('m.oauth_token', $oauth_token);
  $res = $query->execute();
  if ($res->rowCount() == 0) {
    return array();
  }

  $record = $res->fetchAssoc();
  return $record;
}