/**
 * Provide relevant instructions to drag event.
 */
(function($) {
  Drupal.behaviors.rawphenoUploadStage01Behaviours = {
    attach: function (context, settings) {
      // Replace button value from browse to choose your file.
	    $('.droppable-browse-button').text('choose your file');
	  
	    // Drop area element.
	    var dropZone = document.getElementById('droppable-dnd');
	    // Initial/default meassage of the drop area.
	    var dropMessageHTML = $('.droppable-message').html();
	    var dropMessage = $('.droppable-message');
	   
	    // Add corresponding message on mouse event.
	    if (dropZone) {
	      // User drags file into the drop area.
		    dropZone.addEventListener('dragover', function() {
		      dropMessage.css('border','3px dashed #AAAAAA');
          // Create a new instruction to user.
          dropMessage.html('<span class="text-drag">Drop to upload and validate your spreadsheet</span>'); 										
	  	  });
	    
		    // User cancels file drop.
		    dropZone.addEventListener('dragleave', function() {
		      dropMessage.css('border','none');
          // Create a new instruction to user.
          dropMessage.html(dropMessageHTML); 										
		    });
		
		    // User drops file.
		    dropZone.addEventListener('drop', function() {
		      dropMessage.css('border','none');
          // Create a new instruction to user.
          dropMessage.html(dropMessageHTML); 		
          
          if ($('div.file-upload-js-error').length > 0) {
            // AJAX dies or is frozen after an error.
            if ($('div.messages').length > 1) {
              // Remove validation result and drupal error message.
              $('div.messages')[0].remove();
            }
          }
		    });
	    }

      // Remove validation result and error messages as soon
      // as DND receives a file. This is for both drag and drop and
      // using the choose a file link (file browser).
      $(document).ajaxStart(function() {
        // AJAX start.
        if ($('div.messages').length > 1) {
          $('div.messages').remove();
        }
      })
      
      // Always scroll to the validation result window.
      // Validation result to optical center.
      .ajaxComplete(function() { 
        // AJAX complete
         $('html, body').animate({scrollTop: $(".container-header").offset().top}, 300);
      });
      
      //Collapse validation result.
      $('fieldset').click(function() { 
        $('a.fieldset-title').click();
      });
      
      // For security, suppress any alert message showing snippet of code or 
	    // module settings to the user.     
	    alert = function() {
        // Clear dropzone stage and set it to default message.
        $('div.droppable-preview-file').hide();
        dropMessage.html(dropMessageHTML);
        // Create an error message.
        $('<div class="messages error">The specified file is not a valid Microsoft Excel File.</div>').insertAfter('hr');
      };
    }
  };
}(jQuery));


/**
 * Diable the next step button until a success message with an id=rawpheno-upload-successful
 * appears on the page.
 */
(function($) {
  Drupal.behaviors.rawphenoUploadStage01ControlWorkflow = {
    attach: function (context, settings) {
      var submitButton = $('#edit-next-step');
      var successMsg = $('#rawpheno-upload-successful');
        
      if (successMsg.length) {
        submitButton.removeClass('form-button-disabled');
        submitButton.removeAttr('disabled');
      }
      else {
        submitButton.addClass('form-button-disabled');
        submitButton.attr('disabled','disabled');
      }
    }
  };
}(jQuery));