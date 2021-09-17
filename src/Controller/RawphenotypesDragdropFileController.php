<?php
/**
 * @file 
 * Drag and drop callbacks.
 */

namespace Drupal\rawphenotypes\Controller;

use Drupal\file\Entity\File;
use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\File\FileSystemInterface;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpFoundation\JsonResponse;
use Symfony\Component\HttpFoundation\File\UploadedFile;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Defines RawphenotypesDragdropFileController class.
 */

 /**
  * @TODO: save file as an record in backup table. 
  *        test file uploaded against max file upload size limit.
  *        test file uploaded against allowed file extensions.
  *        implement submit function when using older browser or IE.
  */
class RawphenotypesDragdropFileController extends ControllerBase {
  /**
   * Handle drag and drop file upload.
   */
  public function upload(Request $request) {
    // Fetch file uploaded.
    $file = $request->files->get('file');
    $fileid = $request->get('fileId');
    
    $name = $file->getClientOriginalName();
    $extn = $file->getClientOriginalExtension();

    if ($extn == 'xlsx') {
      $destination = \Drupal::service('file_system')
        ->getTempDirectory();
      $file->move($destination, $fileid . '.' . $extn);

      $response = 'File is excel file';
      $error = FALSE;
    }
    else {
      $response = 'File is not an excel file';
      $error = TRUE;
    }

    return new JsonResponse(['response' => $response, 'error' => $error]);  
  }
}