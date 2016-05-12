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
function media_23video_oauth_application_add($consumer_key, $consumer_secret, $oauth_token, $oauth_token_secret) {
  // Don't create if it already exists.
  if (media_23video_oauth_application_count_by_consumer($consumer_key, $consumer_secret, FALSE) > 0) {
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
function media_23video_oauth_application_update($consumer_key, $consumer_secret, $oauth_token, $oauth_token_secret, $domain, $user_id, $user_name, $user_email, $site_id, $site_name, $weight) {

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
      'authorized' => 1,
      'weight' => $weight,
    ));
    $query->condition('consumer_key', $consumer_key);
    $query->condition('consumer_secret', $consumer_secret);
    $query->condition('authorized', 0);
    return $query->execute() ? TRUE : FALSE;
  }
  catch (PDOException $e) {
    drupal_set_message(t('Exception raised while updating application/user entry.'), 'error');
    watchdog('media_23video_oauth', 'Exception raised while updating application/user entry. Message: ' . $e->getMessage(), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Delete application/user entry and related roles from db.
 *
 * @param null $id
 *   ID of entry
 *
 * @return bool
 *   TRUE on success, FALSE otherwise
 */
function media_23video_oauth_application_delete($id = NULL) {
  if (!$id) {
    return FALSE;
  }

  // Delete application/user.
  try {
    $query = db_delete('media_23video_oauth');
    $query->condition('id', intval($id));
    $res = $query->execute();
    if (!$res) {
      return FALSE;
    }
  }
  catch (PDOException $e) {
    drupal_set_message(t('Exception raised while deleting application/user from db.'), 'error');
    watchdog('media_23video_oauth', 'Exception raised while deleting application/user entry. Message: ' . $e->getMessage(), WATCHDOG_ERROR);
    return FALSE;
  }

  // Also delete roles for the deleted application/user.
  try {
    $query = db_delete('media_23video_oauth_roles');
    $query->condition('oauth_id', intval($id));
    $res = $query->execute();
    if (!$res) {
      return FALSE;
    }
  }
  catch (PDOException $e) {
    drupal_set_message(t('Exception raised while deleting role from db.'), 'error');
    watchdog('media_23video_oauth', 'Exception raised while deleting role entry. Message: ' . $e->getMessage(), WATCHDOG_ERROR);
    return FALSE;
  }

  return TRUE;
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
 * @param bool $authorized
 *   Check for authorized or non-authorized applications
 *
 * @return int
 *   Return db count
 */
function media_23video_oauth_application_count_by_consumer($consumer_key, $consumer_secret, $authorized = FALSE) {
  return media_23video_oauth_get_applications(array('consumer_key' => $consumer_key, 'consumer_secret' => $consumer_secret, 'authorized' => $authorized), array(), TRUE);
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
function media_23video_oauth_application_get_by_oauth_token($oauth_token) {
  if (is_null($oauth_token)) {
    return FALSE;
  }

  $records = media_23video_oauth_get_applications(array('oauth_token' => $oauth_token));
  if (!is_null($records) && !empty($records)) {
    return array_shift($records);
  }
  return FALSE;
}

function media_23video_oauth_application_get_by_oauth_id($oauth_id) {
  if (is_null($oauth_id)) {
    return FALSE;
  }

  $records = media_23video_oauth_get_applications(array('id' => $oauth_id));

  if (!is_null($records)) {
    return $records;
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
    $query->orderBy('m.weight', 'ASC');
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

/**
 * Get max application weight.
 *
 * @return int
 *   Max weight of applications
 */
function media_23video_oauth_application_max_weight() {
  $query = db_select('media_23video_oauth');
  $query->addExpression('MAX(weight)');
  try {
    $max = $query->execute()->fetchField();
    return is_null($max) ? 0 : $max;
  }
  catch (PDOException $e) {
    watchdog('media_23video_oauth', t("Couldn't get weight. Message: @message", array('@message' => $e->getMessage())), WATCHDOG_ERROR);
  }
}

/**
 * Get min application weight.
 *
 * @return int
 *   Min weight of applications
 */
function media_23video_oauth_application_min_weight() {
  $query = db_select('media_23video_oauth');
  $query->addExpression('MIN(weight)');
  try {
    $min = $query->execute()->fetchField();
    return is_null($min) ? 0 : $min;
  }
  catch (PDOException $e) {
    watchdog('media_23video_oauth', t("Couldn't get weight. Message: @message", array('@message' => $e->getMessage())), WATCHDOG_ERROR);
  }
}

/**
 * Replace weight of an application with either the previous or next app.
 *
 * @param int $weight
 *   The weight to replace
 *
 * @param string $direction
 *   Which direction to replace weight (up or down)
 */
function media_23video_oauth_application_weight_change($weight, $direction) {
  $new_weight = $direction == 'up' ? $weight - 1 : $weight + 1;
  $query = db_select('media_23video_oauth', 'm');
  $query->fields('m');

  $db_or = db_or();
  $db_or->condition('m.weight', $weight);
  $db_or->condition('m.weight', $new_weight);
  $query->condition($db_or);
  $query->orderBy('m.weight', $direction == 'up' ? 'desc' : 'asc');
  try {
    $res = $query->execute();
    $record_1 = $res->fetchAssoc();
    $record_2 = $res->fetchAssoc();
  }
  catch (PDOException $e) {
    watchdog('media_23video_oauth', t("Couldn't get weights. Message: @message", array('@message' => $e->getMessage())), WATCHDOG_ERROR);
  }

  $query = db_update('media_23video_oauth');
  $query->fields(array(
    'weight' => $new_weight,
  ));
  try {
    $query->condition('id', $record_1['id']);
    $query->execute();
  }
  catch (PDOException $e) {
    watchdog('media_23video_oauth', t("Couldn't update weight. Message: @message", array('@message' => $e->getMessage())), WATCHDOG_ERROR);
  }

  $query = db_update('media_23video_oauth');
  $query->fields(array(
    'weight' => $weight,
  ));
  try {
    $query->condition('id', $record_2['id']);
    $query->execute();
  }
  catch (PDOException $e) {
    watchdog('media_23video_oauth', t("Couldn't update weight. Message: @message", array('@message' => $e->getMessage())), WATCHDOG_ERROR);
  }
}

/**
 * Add role for application.
 *
 * @param int $oauth_id
 *   Application/user id
 *
 * @param int $role_id
 *   Role id
 *
 * @param int $weight
 *   Weight
 *
 * @param bool $weight_auto_inc
 *   Whether to increment weight automatically
 *
 * @return bool
 *   Returns true if role was added
 */
function media_23video_oauth_role_add($oauth_id, $role_id, $weight = NULL, $weight_auto_inc = FALSE) {
  try {
    $query = db_insert('media_23video_oauth_roles');
    $fields = array(
      'oauth_id' => intval($oauth_id),
      'role_id' => intval($role_id),
    );
    if (!is_null($weight)) {
      $fields['weight'] = intval($weight);
    }
    if ($weight_auto_inc) {
      $max_weight = media_23video_oauth_role_max_weight($oauth_id);
      $max_weight++;
      $fields['weight'] = $max_weight;
    }

    $query->fields($fields);
    $res = $query->execute();
    return TRUE;
  }
  catch (PDOException $e) {
    return FALSE;
  }
}

/**
 * Update role, e.g. change weight.
 *
 * @param int $oauth_id
 *   Application/user id
 *
 * @param int $role_id
 *   Role id
 *
 * @param int $weight
 *   Weight
 *
 * @return bool
 *   Return true if role was updated successfully
 */
function media_23video_oauth_role_update($oauth_id, $role_id, $weight = 0) {
  try {
    $query = db_update('media_23video_oauth_roles');
    $query->fields(array(
      'weight' => intval($weight),
    ));
    $query->condition('oauth_id', intval($oauth_id));
    $query->condition('role_id', intval($role_id));
    $res = $query->execute();
    return ($res ? TRUE : FALSE);
  }
  catch (PDOException $e) {
    drupal_set_message(t('Raised exception while updating role'), 'error');
    return FALSE;
  }
}

/**
 * Delete role/roles.
 *
 * @param int $oauth_id
 *   Application/user id
 *
 * @param int $role_id
 *   (optional) Role id. If this is omitted, delete all roles for oauth_id.
 *
 * @return bool
 */
function media_23video_oauth_role_delete($oauth_id, $role_id = NULL) {
  try {
    $query = db_delete('media_23video_oauth_roles');
    $query->condition('oauth_id', intval($oauth_id));
    if ($role_id) {
      $query->condition('role_id', intval($role_id));
    }
    $res = $query->execute();
    return $res ? TRUE : FALSE;
  }
  catch (PDOException $e) {
    return FALSE;
  }
}

/**
 * Get roles sorted by weight.
 *
 * @param int $oauth_id
 *   (optional) OAuth id
 *
 * @param array $role_ids
 *   (optional) Array of role ids
 *
 * @return array
 *   Array of stdClass objects
 */
function media_23video_oauth_role_get($oauth_id = NULL, $role_ids = NULL) {
  $query = db_select('media_23video_oauth_roles', 'm');
  $query->fields('m');
  if (intval($oauth_id)) {
    $query->condition('oauth_id', intval($oauth_id));
  }
  if (intval($role_ids)) {
    $role_ids = array($role_ids);
  }
  if (is_array($role_ids) && !empty($role_ids)) {
    $query->condition('role_id', $role_ids, 'IN');
  }

  $query->leftJoin('role', 'r', 'r.rid = m.role_id');
  $query->addField('r', 'name');

  $query->orderBy('m.weight', 'ASC');

  try {
    $res = $query->execute();
    return $res->fetchAll();
  }
  catch (PDOException $e) {
    return FALSE;
  }
}

/**
 * @param int $oauth_id
 *   Application/user id
 *
 * @param array $role_ids
 *   Array of role ids
 *
 * @return bool|array
 *   Returns associative array on success, FALSE otherwise
 */
function media_23video_oauth_roles_get($oauth_id = NULL, $role_ids = NULL) {
  $query = db_select('media_23video_oauth_roles', 'm');
  $query->fields('m');
  if (intval($oauth_id)) {
    $query->condition('oauth_id', intval($oauth_id));
  }
  if (!empty($role_ids) && !is_array($role_ids)) {
    $role_ids = array($role_ids);
  }
  if (is_array($role_ids) && !empty($role_ids)) {
    $query->condition('role_id', $role_ids, 'IN');
  }

  $query->leftJoin('role', 'r', 'r.rid = m.role_id');
  $query->addField('r', 'name');

  $query->orderBy('m.weight', 'ASC');

  try {
    $res = $query->execute();
    return $res->fetchAllAssoc('role_id');
  }
  catch (PDOException $e) {
    return FALSE;
  }
}

/**
 * Get max wight of roles.
 *
 * @param int $oauth_id
 *   Application/user id
 *
 * @return bool|int
 *   Return int on success, FALSE otherwise
 */
function media_23video_oauth_role_max_weight($oauth_id) {
  $query = db_select('media_23video_oauth_roles');
  $query->addExpression('MAX(weight)');
  $query->condition('oauth_id', $oauth_id);
  try {
    $max = $query->execute()->fetchField();
    return is_null($max) ? 0 : $max;
  }
  catch (PDOException $e) {
    watchdog ('media_23video_oauth', t("Couldn't change weight of role. Message: @message", array('@message' => $e->getMessage())), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Get user id according to logged in user's roles and weight of site settings.
 *
 * @param int $site_id
 *   Site id
 *
 * @return array
 *   Associative array of 23video application
 */
function media_23video_oauth_get_user_id($site_id = NULL) {
  global $user;

  $roles = array_keys($user->roles);

  $query = db_select('media_23video_oauth', 'm');
  $query->leftJoin('media_23video_oauth_roles', 'r', 'm.id = r.oauth_id');
  $query->fields('m');
  $query->fields('r');
  $query->condition('r.role_id', $roles, 'IN');
  if ($site_id) {
    $query->condition('m.site_id', $site_id);
  }
  $query->orderBy('m.weight', 'ASC');
  $query->orderby('r.weight', 'ASC');

  try {
    $res = $query->execute();
    $record = $res->fetchAssoc();
    return $record;
  }
  catch (PDOException $e) {
    drupal_set_message(t("Error when fetching application settings"), 'error');
    watchdog('media_23video_oauth', t("Exception raised while getting application settings. Message: @message", array('@message' => $e->getMessage())), WATCHDOG_ERROR);
  }
}
