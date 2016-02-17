/**
 * @file
 * Manage behaviors in instructions page.
 */
(function($) {
  Drupal.behaviors.rawphenoInstructionsTab = {
    attach: function (context, settings) {
      $(document).ready(function() {
        // IMAGE GALLERY
        // Array to hold all image file.
        var gallery = new Array(0, 1);
		    // Array to hold all relevant captions.
		    var caption = new Array(0, 1);
		    
		    // Gallery elements set 1.
		    gallery[0] = ['01-tendrils-no-elongation', 
		                  '02-tendrils-no-elongation', 
		                  '01-tendrils-elongation'];
		    
		    caption[0] = ['No elongation', 
		                  'No elongation', 
		                  'Elongation'];
		    
		    // Gallery elements set 2.
		    gallery[1] = ['01-pods-emerged', 
		                  '02-pods-emerged', 
		                  '01-pods-variation', 
		                  '02-pods-variation']; 
		    
		    caption[1] = ['Freshly emerged pods', 
		                  'Freshly emerged pods', 
		                  'Sample of the pod variation', 
		                  'Sample of the pod variation'];
		
		    // Path to appendix folder.
        var imgPath = $('#path').val();
		
		    // Attach behavior to Photo Appendix.
		    $('#fragment-5 a').click(function(){
		      // Option select containing topics.
	        var topic = $('#fragment-5 select').val();
	        // Image container - showing default image.
		      var curImg = $('#cur-img').val();
		      
		      // Hold the index of the current image shown.
		      curImg = parseInt(curImg);
		      
          // Determine if user clicks on next or prev link
          // in the image gallery.
          var showImg = ($(this).text() == '<') ? curImg - 1 : curImg + 1;
		      
		      // Replace the image src to show the next or prev image in the topic.
		      if (gallery[topic][showImg]) {
			      $('#fragment-5 img').attr('src', imgPath+gallery[topic][showImg]+'.jpg'); 
			      $('#fragment-5 em').contents().replaceWith(caption[topic][showImg]); 
			      $('#cur-img').val(showImg);
		      }
		    });
	      
	      // When user will change topic, load default or first item in the 
	      // image and caption array.	
		    $('#fragment-5 select').change(function(){ 
	        var topic = $(this).val();
		      $('#fragment-5 img').attr('src', imgPath+gallery[topic][0]+'.jpg');
		      $('#fragment-5 em').contents().replaceWith(caption[topic][0]); 
		      $('#cur-img').val(0);
		    });
		    
		    // SEARCH FUNCTIONALITY
		    // Reference to search textbox.
        var txtField = $('#edit-search');
        
        // Create a label as default value in the search text field.
        // When selected will clear the value.
        txtField.focus(function() {  
        if (this.value == 'Search Trait') { 
          this.value = ''; 
        }}).blur(function() {
          if (this.value == '') {
          this.value = 'Search Trait';
        }});

        // Initialize JQuery Tabs.
        $('#tabs').tabs({delay:0}); 
        
        // Array of traits.
        var availableTrait = new Array();
        
        // Access JSON List of traits created by function callback
        // in instructions page.
        var pathJSON = $('#traits-json').val();
        var objTraits = $.getJSON(pathJSON, function(result) {
          $.each(result, function(i, field) {
            availableTrait[i] = field;
          })
        });
        
        // Reference to Search button.
        var btnSearch = $("#btn_submit");
        btnSearch.click(function() { 
          if (txtField.val() == null || txtField.val().trim() == '') {
            // Field is empty
            alert('Trait field is empty');
          }
          else {
            // Compare user input against the array of traits to see
            // if trait is available.
            if ( btnSearch.val() == 'Search' ) {
              // Get the index of the trait.
              var traitIndex = null;
              for( var i = 0; i < availableTrait.length; i++ ) {
                if (availableTrait[i].toLowerCase() == txtField.val().toLowerCase()) {
                  traitIndex = i;
                  break;
                }
              }
              //
              
              if( traitIndex != null ) {
                // If search is successful - replace Search button value
                // to Clear Search to allow user to reset search.
                btnSearch.val('Clear search');
              
                // Determine which category the trait is in.
                // Include category in search result.
                var traitCategory = '';
                if (traitIndex >= 0 && traitIndex <= 9) {
                  traitCategory = 'Essential Trait';
                } 
                else if (traitIndex >= 10 && traitIndex <= 19) {
                  traitCategory = 'Optional Trait';
                } 
                else {
                  traitCategory = 'Subset Trait';
                } 
                
                // Find the table the trait is in.
                var traitType = '<p>* This trait is '+traitCategory+'</p>';
                var countLi = $('#tabs tr').size();
                for(var x = 0; x < countLi; x++) {
                  var m = $('#tabs tr').eq(x).find('div').text(); 
                  if (m == txtField.val()) {
                    var traitTrIndex = x;
                    break;
                  }
                } 
                
                // When table is found.
                // Copy the table row <tr> with the trait information.
                var traitRow = $('#tabs table').find('tr').eq(traitTrIndex).html();
                var tableHeader = $('table').eq(1).find('tr').eq(0).html();
                var newDiv = traitType+'<table><tr>'+tableHeader+'</tr><tr>'+traitRow+'</tr></table>';
                $('#container-option').append(newDiv);
              }
              else {
                // Not found
                alert('Trait '+txtField.val()+' not found');
              }            
            }
            else {
              // Reset search - remove the result and reset button label.
              btnSearch.val('Search');
              txtField.val('Search Trait');
              // Remove search result.
              $('#container-option table, #container-option p').hide('slow').remove();
            }  
          }  
        });
      });
    }
  };
}(jQuery));
