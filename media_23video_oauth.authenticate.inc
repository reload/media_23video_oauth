<?php

/**
 * Table of 23Video sites.
 */
function media_23video_oauth_admin() {
  $header = array(
    array(
      'data' => 'Weight',
      'field' => 'weight',
      'sort' => 'asc',
    ),
    array(
      'data' => 'Change weight',
    ),
    array(
      'data' => 'Domain',
      'field' => 'domain',
      'sort' => 'asc',
    ),
    array(
      'data' => 'Site name',
      'field' => 'site_name',
      'sort' => 'asc',
    ),
    array(
      'data' => 'User name',
      'field' => 'user_name',
      'sort' => 'asc',
    ),
    array(
      'data' => 'User email',
      'field' => 'user_email',
      'sort' => 'asc',
    ),
    array(
      'data' => 'Consumer key',
      'field' => 'consumer_key',
      'sort' => 'asc',
    ),
    array(
      'data' => 'Roles',
    ),
    array(
      'data' => 'Actions',
    ),
  );
  $rows = array();

  $min_weight = media_23video_oauth_application_min_weight();
  $max_weight = media_23video_oauth_application_max_weight();

  foreach (media_23video_oauth_get_applications(array(), $header) as $id => $application) {
    $roles_o = media_23video_oauth_roles_get($id);
    $roles = array();
    foreach ($roles_o as $role) {
      $roles[] = $role->name;
    }

    $change_weight = array();
    if ($application->weight != $min_weight) {
      $change_weight[] = l(t('Up'), url('admin/config/media/media-23video/oauth-settings/weight/' . $application->weight . '/up'));
    }
    if ($application->weight != $max_weight) {
      $change_weight[] = l(t('Down'), url('admin/config/media/media-23video/oauth-settings/weight/' . $application->weight . '/down'));
    }

    $actions = array(
      l(t('Settings'), url('admin/config/media/media-23video/oauth-settings/' . $id)),
      l(t('Delete'), url('admin/config/media/media-23video/oauth-delete/' . $id))
    );

    $rows[] = array(
      $application->weight,
      implode(' | ', $change_weight),
      $application->domain,
      $application->site_name,
      $application->user_name,
      $application->user_email,
      $application->consumer_key,
      implode(', ', $roles),
      implode(' | ', $actions),
    );
  }

  // Make sortable table.
  $res = theme_table(array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array(),
    'caption' => t('23Video Applications'),
    'colgroups' => array(),
    'sticky' => '',
    'empty' => t('empty'),
  ));

  $res .= l(t('Connect to new application / user (consumer key/secret pair)'), url('admin/config/media/media-23video/oauth'));

  return $res;
}

function media_23video_oauth_weight_change($weight, $direction) {
  media_23video_oauth_application_weight_change($weight, $direction);
  drupal_goto('admin/config/media/media-23video');
}

/**
 * OAuth authentication form.
 */
function media_23video_oauth_authenticate() {
  $form = array();

  $form['consumer']['consumer_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Consumer Key'),
    '#parents' => array('consumer', 'consumer_key'),
  );

  $form['consumer']['consumer_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Consumer Secret'),
    '#parents' => array('consumer', 'consumer_secret'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Go',
  );

  return $form;
}

/**
 * OAuth authentication validation handler.
 */
function media_23video_oauth_authenticate_validate($form, &$form_state) {
  if (!isset($form_state['values']['consumer']['consumer_key']) || empty($form_state['values']['consumer']['consumer_key'])) {
    form_set_error('consumer_key', t('Consumer key must be set'));
  }
  if (!isset($form_state['values']['consumer']['consumer_secret']) || empty($form_state['values']['consumer']['consumer_secret'])) {
    form_set_error('consumer_secret', t('Consumer key must be set'));
  }

  // Check if consumer key/secret pair already exists in db.
  if (media_23video_oauth_application_count_by_consumer($form_state['values']['consumer']['consumer_key'], $form_state['values']['consumer']['consumer_secret'], FALSE) > 0) {
    form_set_error('consumer', t('Consumer key/secret pair already exists'));
  }
}

/**
 * OAuth authentication submit handler.
 */
function media_23video_oauth_authenticate_submit($form, &$form_state) {
  global $base_url;

  $oauth = new OAuth($form_state['values']['consumer']['consumer_key'], $form_state['values']['consumer']['consumer_secret']);
  try {
    $request = $oauth->getRequestToken('http://api.visualplatform.net/oauth/request_token', $base_url . '/admin/config/media/media-23video/oauth-callback');

    if ($request['oauth_callback_confirmed'] == TRUE) {
      media_23video_oauth_application_add($form_state['values']['consumer']['consumer_key'], $form_state['values']['consumer']['consumer_secret'], $request['oauth_token'], $request['oauth_token_secret']);
      $form_state['redirect'] = 'http://api.visualplatform.net/oauth/authorize?oauth_token=' . check_plain($request['oauth_token']);
    }
  }
  catch (OAuthException $e) {
    watchdog('media_23video_oauth', 'Exception raised while requesting OAuth token. Message: ' . $e->getMessage(), WATCHDOG_ERROR);
    form_set_error('consumer', t('Code @code (@last). @message', array(
      '@code' => $e->getCode(),
      '@last' => $e->lastResponse,
      '@message' => $e->getMessage())));
  }
}

/**
 *
 */
function media_23video_oauth_authenticate_callback() {
  global $base_url;
  $params = drupal_get_query_parameters();

  $application = media_23video_oauth_application_get_by_oauth_token($params['oauth_token']);
  if (!$application) {
    drupal_set_message(t("Application/user doesn't exist"), 'error');
    drupal_goto('admin/config/media/media-23video');
  }

  // Create OAuth object and set tokens.
  $oauth = new OAuth($application->consumer_key, $application->consumer_secret);
  $oauth->setToken($application->oauth_token, $application->oauth_token_secret);

  try {
    $response = $oauth->getAccessToken('http://api.visualplatform.net/oauth/access_token', NULL, $params['oauth_verifier']);
  }
  catch (OAuthException $e) {
    drupal_set_message(t('Exception raised while trying to get access token from 23 Video Site. Code @code (@last). @message', array(
      '@code' => $e->getCode(),
      '@last' => $e->lastResponse,
      '@message' => $e->getMessage(),
    )), 'error');
    watchdog('media_23video_oauth', 'Exception: ' . $e->getMessage(), array(), WATCHDOG_ERROR);
  }

  $weight = media_23video_oauth_application_max_weight() + 1;

  media_23video_oauth_application_update($application->consumer_key, $application->consumer_secret, $response['oauth_token'], $response['oauth_token_secret'], $response['domain'], $response['user_id'], $response['user_name'], $response['user_email'], $response['site_id'], $response['site_name'], $weight);

  drupal_goto('admin/config/media/media-23video');
}

function media_23video_oauth_delete($form, &$form_state, $id) {
  $form['application'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );

  $res = confirm_form($form, t('Are you sure?'), 'admin/config/media/media-23video');
  return $res;
}

function media_23video_oauth_delete_submit($form, &$form_state) {
  $id = isset($form_state['values']['application']) ? $form_state['values']['application'] : 0;

  $res = media_23video_oauth_application_delete($id);
  if ($res) {
    drupal_set_message(t('Application successfully deleted'));
  }
  else {
    drupal_set_message(t("Couldn't delete application."), 'error');
    watchdog('media_23video_oauth', t("Couldn't delete application."), WATCHDOG_ERROR);
  }

  $form_state['redirect'] = 'admin/config/media/media-23video';
}
