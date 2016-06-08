/**
 * @file 
 * Manage behavior in backup page
 */
(function($) {
  Drupal.behaviors.rawphenoBackupBehaviours = {
    attach: function (context, settings) {
      $('.droppable-browse-button').text('choose your file');
      
      var window = $('#container-add-project');
      $('#link-add-prj').click(function(event) {
        event.preventDefault();
        
        if (window.is(':visible')) {
          window.slideUp(300);
          $(this).text('Add Project');
        }
        else {
          window.slideDown(300);
          $(this).text('Close Add Project');
        } 
        
        $(this).blur();
      });
      
      
      
      // Drop area element.
	    var dropZone = document.getElementById('droppable-bdnd');
	    // Initial/default meassage of the drop area.
	    var dropMessageHTML = $('.droppable-message').html();
	    var dropMessage = $('.droppable-message');
	    
	    // Add corresponding message on mouse event.
	    if (dropZone) {
	      // User drags file into the drop area.
		    dropZone.addEventListener('dragover', function() {
		      dropMessage.css('border','3px dashed #AAAAAA');
          // Create a new instruction to user.
          dropMessage.html('<span class="text-drag">Drop to backup your spreadsheet</span>'); 										
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
      
      
      
      
    }
  };
}(jQuery));