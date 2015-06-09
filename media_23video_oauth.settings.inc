<?php

function media_23video_oauth_settings($form, &$form_state, $oauth_id) {
  $roles = media_23video_oauth_role_get($oauth_id);

  $form = array(
    '#theme' => 'media_23video_form_table',
    '#header' => array(t('Role name'), t('Weight'), t('Actions')),
  );

  $oauth = media_23video_oauth_application_get_by_oauth_id($oauth_id);
  $oauth = array_shift($oauth);

  $form['before']['name'] = array(
    '#markup' => "<h1>{$oauth->site_name} - {$oauth->user_name} - {$oauth->user_email}</h1>",
  );

  foreach ($roles as $id => $role) {
    $form['roles'][$id][$id . '_first']['name'] = array(
      '#type' => 'markup',
      '#markup' => '<h2>' . $role->name . '</h2>',
      '#parents' => array('roles', $id),
    );
    $form['roles'][$id][$id . '_first']['oauth_id'] = array(
      '#type' => 'hidden',
      '#value' => $role->oauth_id,
      '#parents' => array('roles', $id, 'oauth_id'),
    );
    $form['roles'][$id][$id . '_first']['role_id'] = array(
      '#type' => 'hidden',
      '#value' => $role->role_id,
      '#parents' => array('roles', $id, 'role_id'),
    );
    $form['roles'][$id][$id . '_second']['weight'] = array(
      '#type' => 'textfield',
      '#default_value' => $role->weight,
      '#size' => '3',
      '#parents' => array('roles', $id, 'weight'),
    );
    $form['roles'][$id][$id . '_actions']['delete'] = array(
      '#type' => 'markup',
      '#markup' => l(t('Delete'), url('admin/config/media/media-23video/oauth-settings/' . intval($role->oauth_id) . '/delete/' . intval($role->role_id))),
    );
  }

  $form['before']['add_role'] = array(
    '#type' => 'markup',
    '#markup' => l(t('Add role'), 'admin/config/media/media-23video/oauth-settings/' . intval($oauth_id) . '/add') . '<br />',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function media_23video_oauth_settings_submit($form, &$form_state) {
  if (!empty($form_state['values']['roles'])) {
    $success = TRUE;
    foreach ($form_state['values']['roles'] as $role) {
      $oauth_id = intval($role['oauth_id']);
      $role_id = intval($role['role_id']);
      $weight = intval($role['weight']);

      $res = media_23video_oauth_role_update($oauth_id, $role_id, $weight);
      if ($res === FALSE) {
        drupal_set_message(t("Couldn't update weight for oauth id @oauth_id and role id @role_id", array(
            '@oauth_id' => $oauth_id,
            '@role_id' => $role_id,
          )
        ), 'error');
      }
    }

    if ($success) {
      $form_state['redirect'] = 'admin/config/media/media-23video/oauth-settings/' . intval($oauth_id);
      drupal_set_message(t('Weights updated'));
    }
  }
}

function media_23video_oauth_settings_add($form, &$form_state, $oauth_id) {
  $oauth = media_23video_oauth_application_get_by_oauth_id($oauth_id);
  $oauth = array_shift($oauth);

  $user_roles = user_roles();
  $roles = array_keys(media_23video_oauth_roles_get($oauth_id));
  foreach ($roles as $id) {
    unset($user_roles[$id]);
  }

  $form['oauth_id'] = array(
    '#type' => 'hidden',
    '#value' => $oauth_id,
  );
  $form['oauth'] = array(
    '#markup' => "<h1>{$oauth->site_name} - {$oauth->user_name} - {$oauth->user_email}</h1>",
  );
  $form['role_id'] = array(
    '#type' => 'select',
    '#title' => t('Roles'),
    '#options' => array('' => t('- choose -')) + $user_roles,
  );
  $form['weight'] = array(
    '#type' => 'textfield',
    '#size' => 2,
    '#title' => t('Weight'),
    '#default_value' => media_23video_oauth_role_max_weight($oauth_id) + 1,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

function media_23video_oauth_settings_add_validate($form, &$form_state) {
  if (!isset($form_state['values']['oauth_id']) || !is_numeric($form_state['values']['oauth_id'])) {
    form_set_error('oauth_id', t('Wrong value oauth id'));
  }
  if (!isset($form_state['values']['role_id']) || !is_numeric($form_state['values']['role_id'])) {
    form_set_error('role_id', t('Wrong value role id'));
  }
  if (!isset($form_state['values']['weight']) || !is_numeric($form_state['values']['weight'])) {
    form_set_error('weight', t('Wrong value'));
  }
}

function media_23video_oauth_settings_add_submit($form, &$form_state) {
  $res = media_23video_oauth_role_add($form_state['values']['oauth_id'], $form_state['values']['role_id'], $form_state['values']['weight']);
  if ($res) {
    $form_state['redirect'] = 'admin/config/media/media-23video/oauth-settings/' . intval($form_state['values']['oauth_id']);
    drupal_set_message(t('Role added'));
  }
  else {
    drupal_set_message(t("Error. Couldn't add role"), 'error');
  }
}

function media_23video_oauth_settings_delete($form, &$form_state, $oauth_id, $role_id) {
  $form['oauth_id'] = array(
    '#type' => 'hidden',
    '#value' => $oauth_id,
  );
  $form['role_id'] = array(
    '#type' => 'hidden',
    '#value' => $role_id,
  );

  $res = confirm_form($form, t('Are you sure?'), 'admin/config/media/media-23video/oauth-settings/' . intval($oauth_id) . '/delete');
  return $res;
}

function media_23video_oauth_settings_delete_submit($form, &$form_state) {
  if (!empty($form_state['values']) && !empty($form_state['values']['oauth_id']) && !empty($form_state['values']['role_id']) && !empty($form_state['values']['confirm']) && $form_state['values']['confirm'] == 1) {
    $res = media_23video_oauth_role_delete($form_state['values']['oauth_id'], $form_state['values']['role_id']);
    $form_state['redirect'] = 'admin/config/media/media-23video/oauth-settings/' . intval($form_state['values']['oauth_id']);
    if ($res) {
      drupal_set_message(t('Role deleted'));
    }
    else {
      drupal_set_message(t('Role already deleted'), 'error');
    }
  }
}