<?php
/**
 * @file
 * Contains class definition of RawphenotypesUserService
 */

namespace Drupal\Rawphenotypes\Services;

use Drupal\user\Entity\User;

class RawphenotypesUserService {
  /**
   * Get list of users.
   * 
   * @param $key
   *   String, keywords that make up the name of the user.
   * @param $limit
   *   Integer, limit result set returned. Default to 10 rows.
   * 
   * @return array
   *   Associative array where the key is the user name and value
   *   is string concatenation of name and email address.
   */
  public static function getUserByKeyword($key, $limit = 10) {
    $ids = \Drupal::entityQuery('user')
	    ->condition('status', 1)
	    ->condition('name', '%' . $key . '%', 'LIKE')
      ->range(0, $limit)
	    ->execute();
	
    $users = User::loadMultiple($ids);
    $arr_users = [];
	  foreach($users as $user) {
      $name = $user->get('name')->value;
      $uid = $user->get('uid')->value;
      $mail = $user->get('mail')->value;

      $arr_users[] = [
        'id' => $uid,
        'name' => $name,
        'mail' => $mail
      ];
	  } 
    
    return $arr_users;
  }

  /**
   * Get user.
   * 
   * @param $user_id
   *   Integer, user id number
   * @param $options
   *   User columns required ie. name, email, status, created etc.
   */
  public static function getUser($user_id, $options) {
    $user_profile = [];
    
    if (is_int($user_id) && is_array($options)) {
      $user = User::load($user_id);
          
      if ($user) {
        foreach($options as $col) {
          if (isset($user->get($col)->value)) {
            $user_profile[ $col ] = $user->get($col)->value;
          }
        }
      }
    }

    return $user_profile;
  }

  /**
   * Get user files (backed up files).
   * 
   * @param $project_user_id
   *   Integer, project user id number that maps a user to a project in pheno_project_user table.
   *   THIS IS NOT THE USER ID.
   */
  public static function getUserFiles($project_user_id) {
    $sql = "
      SELECT t1.*, t2.filename, t2.filesize, t2.created, t2.uri
      FROM pheno_backup_file AS t1 INNER JOIN file_managed AS t2 USING(fid)
      WHERE t1.project_user_id = :project_user_id ORDER BY t1.version DESC
    ";
    $args = [':project_user_id' => $project_user_id];
    
    $query = \Drupal::database()
      ->query($sql, $args);
    
    $query->allowRowCount = TRUE;

    return ($query->rowCount()) ? $query->fetchAll() : [];
  }
}