<?php
/**
 * @file 
 * Autocomplete callbacks.
 */

namespace Drupal\rawphenotypes\Controller;

use Drupal\Core\Controller\ControllerBase;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpFoundation\JsonResponse;

/**
 * Defines RawphenotypesAutocompleteController class.
 */
class RawphenotypesAutocompleteController extends ControllerBase {
  /**
   * Get users.
   */
  public function autocompleteUser(Request $request) {
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
  
  /**
   * Get trait.
   */
  public function autocompleteTerm($project_id = NULL, Request $request) {
    $default_service = \Drupal::service('rawphenotypes.default_service');
    $trait_type = $default_service::getTraitTypes();

    $sql = "
      SELECT name
      FROM chado.cvterm RIGHT JOIN pheno_project_cvterm USING(cvterm_id)
      WHERE project_id = :project_id AND type <> :plantprop
    ";
    $args = [':project_id' => $project_id, ':plantprop' => $trait_type['type4']];

    $result = \Drupal::database()
      ->query($sql, $args);
    
    // Array to hold list of traits.
    $arr_headers = [];

    // Determine if keyword is provided.
    if (empty($request)) {
      foreach($result as $n) {
        $arr_headers[] = $n->name;
      }
    }
    else {
      foreach($result as $n) {
        if (str_contains(strtolower($n->name), strtolower($request->query->get('q')))) {
          $arr_headers[ $n->name ] = $n->name;
        }
      }
    }

    return new JsonResponse($arr_headers);  
  }
}