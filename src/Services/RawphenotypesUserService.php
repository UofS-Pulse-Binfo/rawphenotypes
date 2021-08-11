<?php
/**
 * @file
 * Contains class definition of RawphenotypesUserService
 * Class to manage user working on a project.
 */

namespace Drupal\Rawphenotypes\Services;

use Drupal\user\Entity\User;

class RawphenotypesUserService {
  /**
   * Get a user assets - backed up file count, id, active projects
   * 
   * @param $project_user_id
   *   Integer, project user id assigned by module to a user.
   * 
   * @return array
   *   Associative array with the following information
   *   user id, project id (user is assigned), project user id (id assigned by module)
   *   file count (backed up files) and project data count (data uploaded).
   */
  public static function getUserAssets($project_user_id) {
    // Array to hold user assets.
    $arr_project_assets = [];

    $sql = "
      SELECT project_id, uid, project_user_id
      FROM pheno_project_user 
      WHERE project_user_id = :project_user_id LIMIT 1
    ";

    $args = [':project_user_id' => $project_user_id];
    
    $user = \Drupal::database()
      ->query($sql, $args)
      ->fetchObject();

    $arr_project_assets['user_id'] = $user->uid;
    $arr_project_assets['project_id'] = $user->project_id;
    $arr_project_assets['project_user_id'] = $user->project_user_id;

    // Given the project id this user is assigned, test if it has any backup files.
    $sql = "
      SELECT COUNT(file_id) 
      FROM pheno_backup_file
      WHERE project_user_id = :project_user_id
    ";

    $file_count = \Drupal::database()
      ->query($sql, $args)
      ->fetchField();

    $arr_project_assets['project_file_count'] = $file_count;

    // Given the project id this user is assigned, test if it has data.
    $sql = "
      SELECT COUNT(plant_id) FROM pheno_plant_project
      WHERE project_id = (SELECT project_id FROM pheno_project_user WHERE project_id = :project_id AND uid = :user_id)";

    $args = [':project_id' => $user->project_id, ':user_id' => $user->uid];
    $data_count = \Drupal::database()
      ->query($sql, $args)
      ->fetchField();

    $arr_project_assets['project_data_count'] = $data_count;

    return $arr_project_assets;
  }

  /**
   * Get user by username.
   * 
   * @param $username
   *   String, name/username of user account.
   * 
   * @return integer
   *   Id/uid number of user account.
   */
  public static function getUserIdByUsername($username) {
    $username = trim($username);
    $user_id = null;

    $user = \Drupal::entityTypeManager()
      ->getStorage('user')
      ->loadByProperties(['name' => $username]);
  
    if ($user) {
      $user = reset($user);
      $user_id = $user->get('uid')->value;
    }
  
    return $user_id;
  }

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

  /**
   * Dissociate a user from a project.
   * NOTE: This will not delete user from your Drupal system.
   */
  public static function removeUserFromProject($project_user_id) {
    $table = 'pheno_project_user';

    \Drupal::database()
      ->delete('pheno_project_user')
      ->condition('project_user_id', $project_user_id)
      ->execute();
  }
}