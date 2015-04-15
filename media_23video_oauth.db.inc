<?php

/**
 * Add new consumer key, secret, oauth token, secret to db.
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
 * @return bool
 *   Return TRUE on success
 */
function media_23video_oauth_db_add($consumer_key, $consumer_secret, $oauth_token, $oauth_token_secret) {
  // Don't create if it already exists.
  if (media_23video_oauth_db_count_by_consumer($consumer_key, $consumer_secret) > 0) {
    return FALSE;
  }

  try {
    $query = db_insert('media_23video_oauth');
    $query->fields(array(
      'consumer_key' => $consumer_key,
      'consumer_secret' => $consumer_secret,
      'oauth_token' => $oauth_token,
      'oauth_token_secret' => $oauth_token_secret,
    ));
    return $query->execute() ? TRUE : FALSE;
  }
  catch (PDOException $e) {
    drupal_set_message(t('Exception raised while adding new OAuth entry to db.'), 'error');
    watchdog('media_23video_oauth', 'Exception raised while adding new oauth entry. Message: ' . $e->getMessage(), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Update application/user entry with oauth token, oauth token secrets, etc.
 *
 * @param string $consumer_key
 *   Consumer key
 *
 * @param string $consumer_secret
 *   Consumer secret
 *
 * @param string $oauth_token
 *   Updated OAuth token
 *
 * @param string $oauth_token_secret
 *   Updated OAuth token secret
 *
 * @param string $domain
 *   23video domain name
 *
 * @param int $user_id
 *   23video user ID
 *
 * @param string $user_name
 *   23video user name
 *
 * @param string $user_email
 *   23video user email
 *
 * @param int $site_id
 *   23video site ID
 *
 * @param string $site_name
 *   23video site name
 *
 * @return bool
 *   Return TRUE on success
 */
function media_23video_oauth_db_update($consumer_key, $consumer_secret, $oauth_token, $oauth_token_secret, $domain, $user_id, $user_name, $user_email, $site_id, $site_name) {

  try {
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
    return $query->execute() ? TRUE : FALSE;
  }
  catch (PDOException $e) {
    drupal_set_message(t('Exception raised while updating application/user entry.'), 'error');
    watchdog('media_23video_oauth', 'Exception raised while updating application/user entry. Message: ' . $e->getMessage(), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Delete application/user entry from db.
 *
 * @param null $id
 *   ID of entry
 *
 * @return bool
 *   TRUE on success, FALSE otherwise
 */
function media_23video_oauth_db_delete($id = NULL) {
  if (!$id) {
    return FALSE;
  }

  try {
    $query = db_delete('media_23video_oauth');
    $query->condition('id', intval($id));
    $res = $query->execute();
    if ($res) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
  catch (PDOException $e) {
    drupal_set_message(t('Exception raised while deleting application/user from db.'), 'error');
    watchdog('media_23video_oauth', 'Exception raised while deleting application/user entry. Message: ' . $e->getMessage(), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Get count of db entries for consumer key/secret pair.
 *
 * @param string $consumer_key
 *   Consumer key
 *
 * @param string $consumer_secret
 *   Consumer secret
 *
 * @return int
 *   Return db count
 */
function media_23video_oauth_db_count_by_consumer($consumer_key, $consumer_secret) {
  return media_23video_oauth_get_applications(array('consumer_key' => $consumer_key, 'consumer_secret' => $consumer_secret), array(), TRUE);
}

/**
 * Get record from OAuth token to be used in callback function.
 *
 * @param string $oauth_token
 *   OAuth token
 *
 * @return bool|stdClass
 *   Return FALSE if not found or stdClass of record
 */
function media_23video_oauth_db_get_by_oauth_token($oauth_token) {
  if (is_null($oauth_token)) {
    return FALSE;
  }

  $records = media_23video_oauth_get_applications(array('oauth_token' => $oauth_token));
  if (!is_null($records) && !empty($records)) {
    return array_shift($records);
  }
  return FALSE;
}

/**
 * Get application records from db.
 *
 * @param array $params
 *   DB conditions
 *
 * @param array $header
 *   Order by header, used for e.g. theme_table
 *
 * @param bool $count
 *   Set to TRUE if we want the row count instead of records
 *
 * @return bool|int|array
 *   Return FALSE if there was an error or exception. Return int for row count.
 *   If records were found, return array of stdClass.
 */
function media_23video_oauth_get_applications($params = array(), $header = array(), $count = FALSE) {
  $query = NULL;
  if (is_array($header) && !empty($header)) {
    $query = db_select('media_23video_oauth', 'm')->extend('TableSort');
  }
  else {
    $query = db_select('media_23video_oauth', 'm');
  }

  $query->fields('m');

  foreach ($params as $id => $val) {
    $query->condition('m.' . check_plain($id), $val);
  }

  if (is_array($header) && !empty($header)) {
    $query->orderByHeader($header);
  }
  else {
    $query->orderBy('m.site_id', 'ASC');
  }

  try {
    $res = $query->execute();
    if ($count) {
      return $res->rowCount();
    }
    return $res->fetchAllAssoc('id');
  }
  catch (PDOException $e) {
    drupal_set_message(t('Exception raised while getting applications from db.'), 'error');
    watchdog('media_23video_oauth', 'Exception raised while getting applications from db. Message: ' . $e->getMessage(), WATCHDOG_ERROR);
    return FALSE;
  }
}
