<?php
/**
 * @file 
 * Autocomplete callback - users
 */

namespace Drupal\rawphenotypes\Controller;

use Drupal\Core\Controller\ControllerBase;

/**
 * Defines RawphenotypesAutocompleteUserController class.
 */
class RawphenotypesAutocompleteUserController extends ControllerBase {
  /**
   * Get users.
   */
  public function result(Request $request) {
    // Load users service.
    $user_service = \Drupal::service('rawphenotypes.user_service');
    $users = $user_service::getUserByKeyword($request);

    $arr_user = [];
    if (count($users)) {
      foreach($users as $user) {
        $arr_user[ $user['name'] ] = $user['name'] . ' - ' . $user['mail'];
      }
    }

    return new JsonResponse($arr_user);
  }
}