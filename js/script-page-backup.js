/**
 * @file
 * Manage behavior in backup page
 * credits to: https://css-tricks.com/drag-and-drop-file-uploading/.
 */
 (function($) {
  Drupal.behaviors.rawphenoBackupBehaviours = {
    attach: function (context, settings) {      
      var dragdropFile = drupalSettings.rawphenotypes.vars.dragdropfile;
      // Reference form element.
      var rawphenotypesForm = $('#rawphenotypes-form');
      // Reference to the main drop zone area element.
      var dropZone = $('.drop-zone');
      // Variable to hold filename picked up by the drop zone.
      var dropFilename = false;
      // File uploaded through the drag and drop.
      var dropFile = false;

      // Reference container for filename.
      var dropFileContainer = $('#drop-zone-file');
      // Reference file field.
      var inputFileField = $('#field-file');
      

      // Establish if browser can support drag and drop.
      var isAdvancedUpload = function() {
        var div = document.createElement('div');
        return (('draggable' in div) || ('ondragstart' in div && 'ondrop' in div)) && 'FormData' in window && 'FileReader' in window;
      }();

      
      // The browser supports drag and drop. A conventional way of
      // selecting file (choose file link) will be available if it were unsupported.
      if (isAdvancedUpload) {
        // Add class to indicate that the drop zone can support drag and drop.
        // This is white container element when unsupported, matching other field elements.
        dropZone.addClass('has-dragdrop-upload');
        
        // Listen for events.
        dropZone.once()
          .on('drag dragstart dragend dragover dragenter dragleave drop', function(e) {
            e.preventDefault();
            e.stopPropagation();
            setMessage('');
          })
          .on('dragover dragenter', function() {
            // Activate drop zone to indicate that drag and drop
            // is active and the element can take the file dragged into it.
            dropZone.addClass('is-dragover')
          })
          .on('dragleave dragend drop', function() {
            // Revert back to the original state.
            dropZone.removeClass('is-dragover')
          })
          .on('drop', function(e) {
            // Set the filename to indicate drop zone has a file.
            // Only when the thing drag and drop is a file.
            if (e.originalEvent.dataTransfer.items[0].kind === 'file') {             
              dropFilename = e.originalEvent.dataTransfer.files[0].name;
              markupDropzoneFile(dropFilename);

              // File to upload using the drag and drop.
              dropFile = e.originalEvent.dataTransfer.files;
              rawphenotypesForm.trigger('submit');
            }            
          });
      }
      
      var submitForm = 0;
      $('#backup-submit-form').once().click(function() {
        submitForm = 1;
      });

      // Handle submit.
      rawphenotypesForm.once()
        .on('submit', function(e) {        
          // Submit when there is file upload.
          if (isAdvancedUpload && dropFile.length > 0) {
            if (submitForm == 0) { 
              e.preventDefault();   
              e.stopPropagation();

              // File data container.
              var ajaxData = new FormData();
              
              if (dropFile) {
                // Append file data container with the file.
                ajaxData.append('file', dropFile[0]);    
                setFileId();       
                ajaxData.append('fileId', getFileId());    
              }
              
              $.ajax({
                url: dragdropFile,
                type: rawphenotypesForm.attr('method'),
                data: ajaxData,
                dataType: 'json',
                cache: false,
                contentType: false,
                processData: false,
                complete: function() {
                  // $form.removeClass('is-uploading');
                },
                success: function(data) {
                  if (data.error) {
                    setMessage(data.response);
                  }
                },
                error: function() {}
              });
            } else {
              // ajax for legacy browsers
            }
          }
        });

      // Whether or not drag and drop is supported, manual upload
      // will be available to user.
      inputFileField.once()
        .on('change', function(e) {
          setMessage('');
          
          if(this.files && this.files.length == 1) {
            dropFilename = e.target.value.split('\\').pop();
          }

          if(dropFilename) {
            markupDropzoneFile(dropFilename);
            dropFile = [ $('#field-file')[0].files[0] ];
            rawphenotypesForm.trigger('submit');
          }
        });

      // When drop zone has file - a remove link is available.
      // Listen to when this link is clicked.
      var hasFile = dropFileContainer.has('a');
      if (hasFile) {
        dropFileContainer.on('click', function(e) {
          e.preventDefault();
          
          setMessage('');
          markupDropzoneFile();
          $('#field-file').val('');
        }); 
      }

      /**
       * Create a markup for the file set in the drop zone element.
       * Markup will include remove link to remove file from the drop zone.
       * @param filename 
       *   Filename picked up by the drop zone element.
       */
      function markupDropzoneFile(filename = null) {
        // Markup that will show the filename provided in the drop zone or using
        // the conventional way to add file plus this markup to remove the file.
        var removeFile = '&nbsp;<a href="#"><small>[remove]</small></a>';
        
        var markup = (filename) ? filename.trim() + removeFile : '';
        dropFileContainer.html(markup);
      }

      /**
       * Create a file id.
       */
      function setFileId() {
         var id = Date.now();
         $('#field-file-id').val(id.toString());
      }
      
      /**
       * Get file id.
       */
      function getFileId() {
        return $('#field-file-id').val();
      }

      /**
       * Set status (error) message.
       */
      function setMessage(message = '') {
        $('.dragdrop-message').html(message);
      }
    }
  };
}(jQuery));