<?php

/**
 * Admin settings form.
 */
function media_23video_oauth_admin() {
  $header = array(
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
  );
  $rows = array();

  foreach (media_23video_oauth_get_applications(array(), $header) as $id => $application) {
    $rows[] = array(
      $application->domain,
      $application->site_name,
      $application->user_name,
      $application->user_email,
      $application->consumer_key,
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
  if (media_23video_oauth_db_count_by_consumer($form_state['values']['consumer']['consumer_key'], $form_state['values']['consumer']['consumer_secret']) > 0) {
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
      media_23video_oauth_db_add($form_state['values']['consumer']['consumer_key'], $form_state['values']['consumer']['consumer_secret'], $request['oauth_token'], $request['oauth_token_secret']);
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

  $application = media_23video_oauth_db_get_by_oauth_token($params['oauth_token']);
  if (!$application) {
    drupal_set_message(t("Application/user doesn't exist"), 'error');
    drupal_goto('admin/config/media/media-23video');
  }

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

  media_23video_oauth_db_update($application->consumer_key, $application->consumer_secret, $response['oauth_token'], $response['oauth_token_secret'], $response['domain'], $response['user_id'], $response['user_name'], $response['user_email'], $response['site_id'], $response['site_name']);

  drupal_goto('admin/config/media/media-23video');
}