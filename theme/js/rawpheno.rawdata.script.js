/**
 * @file
 * Create graphical representation of phenotypic data.
 */
(function($) {
  Drupal.behaviors.rawphenoRawDataCreateHeatMap = {
    attach: function (context, settings) {
      ////
      // Variable to hold the default option when a category is selected.
      var categoryOptionLocation, categoryOptionYear;

      // Add event listener to window resize.
      // Reposition elements with the new window width.
      d3.select(window).on('resize', render);

      // Add event listener to select fields.
      $('#container-form-select select').change(function(i) {
        // Clear warning message.
        if ($('.warn-message')) {
          $('.warn-message').remove();
        }

        // Remove previous barchart categorize options and start with a new set of selectboxes.
        if($('#chart-control-container')) {
          // Remove container and everything inside it (children).
          $('#chart-control-container').remove();
        }

        // Determine which select box was changed.
        var selectID = i.target.id;

        // UPDATE HEATMAP OR DISPLAY BARCHART.
        // When user select a project.
        if (selectID == 'rawdata-sel-project') {
          // Start loading animation when user changes project.
          loading('heatmap');

          // Reset the select trait default to the word
          // Select a trait to highlight in the chart.
          $('select[name=rawdata-sel-trait] option:first-child').attr('selected', 'selected');

          // Clear the chart before visualizing a dataset.
          $('g').remove();

          // Start the heat map chart.
          // Read JSON data for heat map.
          var project_id = i.target.value;
          heatmapFile = file + '/rawdata/?project_id=' + project_id;
          d3.json(heatmapFile, function(error, data) {
            if (error) {
              // Error reading JSON.
              throw error;
            }
            else if(data == 0) {
              // No data.
              noData('.data-chart');
            }
            else {
              // Initialize heat map chart by adding all elements (rect, g, text, etc.) into the DOM,
              // and then call render function and position these elements in the right place.
              initializeHeatmapChart(data);
            }
          });
        }
        else {
          var project_id = $('#rawdata-sel-project').val();
          var traitId = i.target.value;
          var traitSelectedName = $('#' + i.target.id + ' option:selected').text();

          // Mark rep where trait was measured.
          markRep(project_id, traitId, traitSelectedName);
          // Start loading animation when user changes project.
          loading('barchart');

          // Extract the available location and year for this particular trait.
          // The values will be used to populate select boxes.
          var categoryFile = file + 'rawdata_trait_category' + '?project_id=' + project_id + '&trait_id=' + traitId;

          d3.json(categoryFile, function(error, categoryData) {
            if (error) {
              // Error reading JSON.
              throw error;
            }
            else if(categoryData == 0) {
              // No data.
              noData('.bar-chart');
            }
            else {
              // Default the barchart to the first entry/option in the select box.
              var defaultYear = categoryData.planting_date[0];

              // Render bar chart.
              // Default to location and a year - first in the list.
              var barchartFile = file + 'rawdata_trait' + '?project_id=' + project_id + '&trait_id=' + traitId + '&category=location' + '&option=' + defaultYear;

              d3.json(barchartFile, function(error, barchartData) {
                if (error) {
                  // Error reading JSON.
                  throw error;
                }
                else if(barchartData == 0) {
                  // No data.
                  noData('.bar-chart');
                }
                else {
                  // Initialize barchart by adding all elements (rect, g, text, etc.) into the DOM,
                  // and then call render function and position these elements in the right place.
                  initializeBarChart(barchartData, categoryData, project_id, traitId);
                }
              });
            }
          });
        }
      });

      // Leaf path/symbol used to mark a rep.
      var leaf = 'm2.46906,17.20495c-0.42208,-1.86678 -3.37632,-8.39646 3.06637,-10.19122c6.44269,-1.79476 9.1144,-2.35212 11.17243,-3.20865c2.05803,-0.85653 5.22901,-3.1601 5.15685,-3.77696c-0.07214,-0.61685 1.40663,9.13151 -0.30335,14.13307c-1.70998,5.00156 -5.52874,7.53668 -10.36077,7.28237c-4.83199,-0.25432 -5.67973,-1.27159 -6.61222,-1.10203c-0.9325,0.16953 -1.56016,2.17883 -2.03454,3.39088c-0.47435,1.21206 -0.59338,1.35635 -0.50862,1.35635c0.08476,0 -1.78022,-1.01725 -2.03454,-1.35635c-0.25431,-0.33909 3.98429,-6.86654 8.98585,-8.64676c5.00161,-1.78023 7.19302,-4.28732 7.88385,-5.51021c0.69078,-1.22288 1.52588,-3.13656 1.52588,-3.17895c0,0.04239 -1.15433,3.08336 -3.6452,4.36577c-2.49086,1.2824 -7.79905,2.96701 -9.91833,4.66247c-2.11929,1.69544 -1.95157,3.647 -2.37363,1.78021l-0.00001,0z';

      // Tooltip/information box.
      var infoBox = d3.select('body')
        .append('div')
        .attr('class', 'tool-tip')
        .style('opacity', 0);

      // Div container of svg canvas.
      // The height and width of the chart canvas is based on the height and width of this
      // HTML element (DIV).
      var divChartContainer = d3.select('#container-rawdata');

      // Heatmap colour code.
      var color = ['#FFFFFF', '#E2EFDA', '#C6E0BA', '#A9D08E', '#70AD47', '#548235', '#375623'];

      // List of traits measured in a rep.
      var traitsList = [];

      var dataSet = {}, chartDimension = {}, rectDimension = {};

      // Canvas width and height;
      var width, height, margin = {};

      // Chart margins.
      // NOTE: THIS MARGIN IS FOR BOTH HEATMAP AND HISTOGRAM!
      margin.top = 40;
      margin.left = 90;
      margin.bottom = 80;
      margin.right = 30;

      // Use this variable to increase or decrease
      // the current margin bottom value and not affect
      // the margin for the histogram.
      // @see note above.

      // Adjusted margin bottom to accommodate
      // text wrapping of long location value.
      marginAdjust = 30;

      // x axis scale.
      var x0, xAxis;

	    // Height of the chart defined in the css rule for #container-rawdata.
      height = parseInt(divChartContainer.style('height'), 10);
      chartDimension.height = height - margin.top - (margin.bottom + marginAdjust);

      // Main svg canvas of the heat map.
      var svg;

	    // Barchart variables
	    // Main the svg canvas of the bar chart.
	    var bsvg;
	    var bx0, bxAxis, bins, dataByBins, dataByTitles, traitSelectedId, traitSelectedName;
	    // 10 px either side of a bin set (10X2)
      var gutter = 20;
      var barchartColor = d3.scale.category20b().domain([0, 20]);
      var categorize;

      // Option to clear the chart.
      $('#container-marker-information, #container-marker-information a').click(function(event) {
        event.preventDefault();

        // Close window.
        infoWindow('off');
        // Remove all markers.
        delMarker('');
        // Remove barchart.
        $('#container-barchart').remove();
      });

      // URL to knowpulse.
      var file = $('#rawdata-json').val();

      // Start loading animation upon initial load of the page.
      loading('heatmap');

      // Start the heat map chart default to the first project in the select a
      // project select box.
      // Read JSON data for heat map.
      var project_id = $('#rawdata-sel-project').val();
	    heatmapFile = file + '/rawdata/?project_id=' + project_id;
	    d3.json(heatmapFile, function(error, data) {
		    if (error) {
          // Error reading JSON.
          throw error;
        }
        else if(data == 0) {
          // No data.
          updateWidth();
          noData('.data-chart');
        }
        else {
          // Initialize heat map chart by adding all elements (rect, g, text, etc.) into the DOM,
          // and then call render function and position these element in the right place.
          initializeHeatmapChart(data);
        }
      });


      // Function to add all heat map chart elements into the DOM.
      // The height attribute or property of the elements remains constant whereas the
      // width is to be determined by the render function.
      function initializeHeatmapChart(data) {
        // Remove loading animation.
        $('.win-loading').remove();

        // Main svg canvas.
        svg = d3.select('.data-chart');
        svg.attr('height', height);

        // CHART DATA
        // Group data with location as primary key.
        // year as secondary key and rep as tertiary.
        // data /location/year/rep - values.
        var n = 0;
        traitsList = [];

        dataByLocation = d3.nest()
          .key(function(d) { return d.location; })
          .key(function(d) { return d.year; })
          .sortKeys(d3.descending)
          .key(function(d) { return d.rep; })
          .sortKeys(d3.sort)
          .rollup(function(v) {
            // Splice the data array values specific to a particular location, year and rep.
            var objEntry = v[0];

            n++;
            // This array associates an id (a rect) to list of traits the total count is based.
            traitsList[n] = objEntry.type_id;

            return {
              'count': objEntry.trait,
              'id': 'obj-' + n.toString()
            }
          })
          .entries(data);

        // All year.
        var dataByYear = d3.nest()
          .key(function(d) { return d.year })
          .sortKeys(d3.descending)
          .entries(data);

        // All reps.
        var dataByRep = d3.nest()
          .key(function(d) { return d.rep; })
          .entries(data);

        // Count number of location, year and rep.
        dataSet.countLocation = dataByLocation.length;
        dataSet.countYear = dataByYear.length;
        dataSet.countRep = dataByRep.length;

        // CHART ELEMENTS
        // Titles and legend
        heatmapChartInfo();

        // Scales
        // X axis (locations)
        x0 = d3.scale.ordinal();
        xAxis = d3.svg.axis().orient('bottom');

        // Y axis (year)
        var y0 = d3.scale.ordinal()
          .rangeRoundBands([0, chartDimension.height]);

        var yAxis = d3.svg.axis()
         .scale(y0)
         .orient('left');

        // Main chart wrapper. Add g element on position
        // x = margin.left and y = margin.top
        var chartWrapper = svg.append('g')
          .attr('id', 'g-chart-wrapper')
          .attr('transform', 'translate(' + margin.left + ',' + margin.top + ')');

        // Each location, add a g container to hold a set of reps and year.
        var locationWrapper = chartWrapper
          .selectAll('g')
          .data(dataByLocation).enter()
          .append('g')
            .attr('class', 'g-each-location')
            .attr('id', function(d, i) { return 'location-' + i; });

        // Each year, add a g container to hold a set of reps in a year.
        rectDimension.height = Math.round(chartDimension.height/dataSet.countYear);
        var yearWrapper = locationWrapper.selectAll('g')
          .data(function(d) { return d.values; }).enter()
          .append('g')
            .attr('class', 'g-each-year')
            .attr('transform', function(d, i) {
              return 'translate(0, '+ (i * rectDimension.height) +')';
            });

        // In g year container, add the reps (rect).
        yearWrapper.selectAll('rect')
          .data(function(d) { return d.values; }).enter()
          .append('g')
          .attr('id', function(d) { return d.values.id; })
          .attr('class', 'g-each-rep')

          .on('mousemove', function(d) {
            d3.select(this).style('opacity', 0.5);
            infoBox.transition().style('opacity', 1);
            infoBox
              .html('Rep ' + d.key + ': '+ d.values.count + ' Traits')
              .style('left', (d3.event.pageX + 10) + 'px')
              .style('top', (d3.event.pageY) + 'px');
          })
          .on('mouseout', function(d) {
            d3.select(this).style('opacity', 1);
            infoBox.transition().style('opacity', 0);
          })

          .append('rect')
          .attr('class', 'rect-each-rep')
          .attr('fill', function(d) {
            var c = setColour(parseInt(d.values.count));
            return color[c];
          })
          .attr('height', rectDimension.height)
          .attr('y', 0);

        // With all reps in place, wrap in a rect with thick border to visually
        // group a location with its reps.
        locationWrapper.append('rect')
          .attr('class', 'rect-wrap-each-location')
          .attr('height', rectDimension.height * dataSet.countYear);

        // Finally, add scales
        // x axis (location).
        x0.domain(dataByLocation.map(function(d) { return d.key; }));
        chartWrapper.append('g')
          .attr('id', 'g-x-axis')
          .attr('class', 'axis')
          .attr('transform', 'translate(0,' + rectDimension.height * dataSet.countYear +')');

        // y axis (year).
        y0.domain(dataByYear.map(function(d) { return d.key; }));
        chartWrapper.append('g')
          .attr('class', 'axis')
          .attr('transform', 'translate(1,0)')
          .call(yAxis);


        // Render heat map chart elements into the right place.
        render();
      }
      // End initialize heat map chart.

      // Function to add all bar map chart elements into the DOM.
      // The height attribute or property of the elements remains constant whereas the
      // width is to be determined by the render function.
      function initializeBarChart(data, dataCategory, pId, tId) {
        // Remove loading animation.
        $('.win-loading').remove();

        // Add filter/categorize controls only when none is available in the DOM.
        if ($('#chart-control-container').length <= 0) {
          // Add the container.
          $('#container-barchart')
            .prepend('<div id="chart-control-container">Categorize by: </div>');

          // Add select boxes.
          // Select a category.
          var selCategory = $('<select id="sel-barchart-category">')
            .appendTo('#chart-control-container');

          // Populate select a category.
          selCategory.append($('<option>').attr('value', 'location').text('Location'));
          selCategory.append($('<option>').attr('value', 'year').text('Year'));
          //

          // Add select option given a select category.
          $('<select id="sel-barchart-option">').appendTo('#chart-control-container');
          // On each new trait, default the select set to location and the first option
          // in the select option select box.
          barchartFillSelect('location', dataCategory);

          // Once the element is in the DOM, attach an event listener to them.
          var catSel = $('#container-barchart select');
          catSel.change(function() {
            // Read the values in the select box and process accordingly.
            var selValues = [];
            catSel.each(function(i) {
              selValues[i] = $(this).val();
            });

            // Store the value of select category.
            // Either by location or year.
            categorize = selValues[0];

            var selId = $(this).attr('id');
            // Category value, Option value, the select box changed and the category data.
            barchartCategorize(selValues[0], selValues[1], selId, dataCategory, pId, tId);
          });
        }

        // Add all barchart elements.
        bsvg = d3.select('.bar-chart')
          .attr('height', height - 30);

        var wrapper = bsvg.append('g')
          .attr('id', 'g-main-barchart-container')
          .attr('transform', 'translate(' + (margin.left + gutter) + ',' + margin.top + ')');

        // CHART DATA.
        // Titles / data.
        dataByTitles = d3.nest()
          .key(function(d) { return d.title; })
          .sortKeys(d3.ascending)
          .entries(data.data);

        // Bins.
        bins = data.bin;

        // Count the number of rows per title in the same bin.
        // Bin / title / count stocks in the same bin.
        dataByBins = d3.nest()
          .key(function(d) { return d.bin; })
          .key(function(d) { return d.title; })
          .rollup(function(v) {
            return v.length;
          })
          .entries(data.data);

        var allCount = dataByBins.map(function(d) {
          var c = d.values.map(function(d) { return d.values; });
          return c;
        });
        allCount = d3.merge(allCount);

        // CHART ELEMENTS.
        // Titles and legend
        barChartInfo(categorize);

        // X axis scale.
        bx0 = d3.scale.ordinal().domain(bins);
        bxAxis = d3.svg.axis()
          .orient('bottom')
          .scale(bx0);

        // Y axis scale.
        // From the stock count, get the maximum.
        var maxCount = d3.max(allCount);

        var by0 = d3.scale.linear()
          .domain([0, maxCount])
          .range([chartDimension.height + marginAdjust, 0])
          .nice();


        // Compute the number of tick marks.
        var noTickMark;
        if (maxCount >= 1 && maxCount <= 10) {
          // Use the max number when maxCount is in range 1 to 10
          noTickMark = maxCount;
        }
        else {
          // Let d3 use scale to generate all tick values.
          noTickMark = null;
        }

        var byAxis = d3.svg.axis().orient('left')
          .scale(by0)
          .ticks(noTickMark)
          .tickFormat(d3.format('d'));
        //

       // Add grid.
       wrapper.selectAll('line.y')
         .data(by0.ticks())
         .enter()
         .append('line')
         .attr('class', 'grid-lines')
         .attr('class', 'y')
         .attr('x1', -20)
         .attr('y1', by0)
         .attr('y2', by0)
         .style('stroke', '#333333')
         .style('opacity', 0.1)
         .attr('shape-rendering', 'crispEdges')
         .attr('stroke-width', '1px');

        // Bar chart.
        // Add g container for each bin. Apply gutter/space to adjacent bins.
        // This will group elements into bins.
        wrapper.selectAll('g')
          .data(bins)
          .enter()
          .append('g')
            .attr('class', 'g-each-bin')
            .attr('id', function(d, i) {
              return 'bin-' + i;
            });

        // In each bin container, add same number of bars representing each title
        // present in the data. When a title is not present, add a space to maintain
        // equal order of title per bin.
        var title = dataByTitles.map(function(d) { return d.key; });

        bins.forEach(function(bin, bin_i) {
          title.forEach(function(title, title_i) {

            var stockCount = +getStockCount(bin, title);
            var h = by0(stockCount);

            d3.select('#bin-' + bin_i)
              .append('g')

            .on('mousemove', function(d) {
              d3.select(this).style('opacity', 0.5);
              infoBox
                .transition()
                .style('opacity', 1);
              infoBox
                .html(title +' ('+stockCount+' stocks)')
                .style('left', (d3.event.pageX + 10) + 'px')
                .style('top', (d3.event.pageY) + 'px');
            })
            .on('mouseout', function(d) {
              d3.select(this).style('opacity', 1);
              infoBox.transition().style('opacity', 0);
            })

            .append('rect')
            .attr('class', 'rect-each-bar')
              .attr('fill', barchartColor(title_i))
              .attr('y', h)
              .attr('height', (chartDimension.height - h) + marginAdjust);
          });
        });

        // X axis.
        bsvg.append('g')
          .attr('id', 'barchart-x-axis')
          .attr('class', 'barchart-axis')
          .attr('transform', 'translate(' + (margin.left + 7) + ',' + (height - margin.bottom) + ')')
          .call(bxAxis);

        // Y axis.
        bsvg.append('g')
          .attr('class', 'barchart-axis')
          .attr('transform', 'translate(' + (margin.left + 7) + ', ' + margin.top +')')
          .call(byAxis);

        // Add chart title
        d3.select('#container-barchart')
          .append('h2')
			    .text('Figure: Distribution of ' + traitSelectedName + ' across Germplasm Phenotyped');

        d3.select('#container-barchart')
          .append('div')
          .attr('class', 'messages warning warn-message')
          .html('The following chart uses raw data and, as such, <em>should never be used in publication and presentation</em>. It is meant to give you a quick visual and to identify problems such as outliers to aid you in your analysis.');

        // Position all bar chart elements into the right place.
        renderBarChart();
      }
      // End initialize bar chart.



      // Render function to position chart elements in the svg canvas.
      // Function update stage with the current window width.
      function updateWidth() {
        // This is the width of the svg container.
        var containerWidth = divChartContainer.style('width');
        // Update the width to be used in redering the chart elements.
        width = parseInt(containerWidth, 10);
      }

      // Function to render/position chart elements.
      function render() {
        // Get the width of the window before rendering chart elements.
        updateWidth();

        // Store the window width/size - use the value in computing
        // the required bin size base on the given screen width available
        // for visualization.
        storeScreenWidth(width);

        // Render heatmap.
        renderHeatmap();

        // Render barchart.
        // If barchart is present
		    if ($('#container-barchart').length > 0) {
		      renderBarChart();
		    }

        // Remove all markers.
        delMarker('');
        // Close all windows.
        infoWindow('off');
        // Remove the bar chart.
        d3.selectAll('#container-barchart').remove();
      }

      // Render heat map chart elements.
      function renderHeatmap() {
        // Adjust the width of svg canvas.
        svg.attr('width', width);

        // The width of the main chart.
        chartDimension.width = width - margin.left - margin.right;

        // Compute width of chart elements.
        var gLocationWrapperWidth = Math.round(chartDimension.width/dataSet.countLocation);
        rectDimension.width = Math.round(gLocationWrapperWidth/dataSet.countRep);

        // Render g elements for each location.
        // Render rect elements for each rep and wrapper for each location.
        d3.selectAll('.g-each-location')
          .attr('transform', function(d, i) {
            return 'translate('+ (i * gLocationWrapperWidth) +', 0)';
          });

        d3.selectAll('.rect-wrap-each-location')
          .attr('width', gLocationWrapperWidth);

        var x = 0;
        d3.selectAll('.rect-each-rep')
          .style('opacity', 0)
          .transition()
          .duration(function() { return randomNumber(); })
          .ease('back')
          .style('opacity', 1)
          .attr('width', rectDimension.width)
          .attr('x', function(d, i) {
            x = (i%dataSet.countRep == 0) ? 0 : x + 1;
            return x * rectDimension.width;
          });

        // Render chart title (keep center) and x axis.
        d3.select('#standard-chart-title')
          .attr('transform', function() {
            return 'translate('+ Math.round(width/2) +', 15)';
          });

        // Render locations title.
        d3.select('#location-title')
          .attr('transform', function() {
            return 'translate('+ Math.round(width/2) + ', ' + (height - margin.top) +')';
          });

        // Render scales
        x0.rangeRoundBands([0, chartDimension.width]);
        xAxis.scale(x0);
        d3.select('#g-x-axis')
          .call(xAxis)
          .selectAll('text')
            .call(wrapWords);
      }

      // Render bar chart elements.
      function renderBarChart() {
        // Less 125px to accommodate legend.
        var legendArea = 125;

        // Adjust the width of svg canvas.
        bsvg.attr('width', width);

        var gBinWidth = Math.round((chartDimension.width - legendArea) / bins.length) - gutter;

        // render bins
        d3.selectAll('.g-each-bin')
          .attr('transform', function(d, i) {
            return 'translate('+ ((gBinWidth + gutter) * i) +', 0)';
          });

        var barWidth = Math.round(gBinWidth / dataByTitles.length);

        var x = 0;
        d3.selectAll('.rect-each-bar')
          .style('opacity', 0)
          .transition()
          .duration(function() { return randomNumber(); })
          .ease('back')
          .style('opacity', 1)
          .attr('width', (barWidth - 2))
          .attr('x', function(d, i) {
            x = (i%dataByTitles.length == 0) ? 0 : x + 1;
            return x * (barWidth + 1);
          });

        // Render scales
        bx0.rangeRoundBands([0, chartDimension.width - legendArea]);
        bxAxis.scale(bx0);
        d3.select('#barchart-x-axis').call(bxAxis);

        // Render Legend
        d3.select('#barchart-legend-rect')
          .attr('transform', 'translate('+ (width - margin.left - margin.right - 25) +', '+ margin.top +')');

        d3.select('#barchart-legend-text')
          .attr('transform', 'translate('+ (width - margin.left - margin.right - 25) +', '+ (margin.top + 10) +')');

        // Render x caption
        d3.select('#x-caption')
          .attr('transform', function() {
            return 'translate(' + Math.round(width/2) + ', '+ (height - margin.bottom + 40) +')';
          })

        // Render grids.
        d3.selectAll('line.y')
          .attr('x2', chartDimension.width - margin.right - margin.left - gutter);
      }



      // Label, caption, legend
      // Heat map
      function heatmapChartInfo() {
        var chartTitle = 'Number of Trials per Location';
        var chartXTitle = 'Locations';
        var chartYTitle = 'Growing Season (year)';
        var chartLegendTitle = 'Number of traits';

        // Title
        var captions = svg.append('g').attr('id', 'g-captions');
        captions.append('text')
          .attr('id', 'standard-chart-title')
          .attr('class', 'chart-title')
          .text(chartTitle);

        // Growing season - y axis
        captions.append('text')
          .attr('class', 'chart-axes')
          .attr('transform', function() {
            return 'translate(35, '+ Math.round(height/2) +') rotate(-90)';
          })
          .text(chartYTitle);

        // Location - x axis.
        captions.append('text')
          .attr('id', 'location-title')
          .attr('class', 'chart-axes')
          .text(chartXTitle);

        // Legend.
        var legend = svg.append('g')
          .attr('id', 'g-legend')
          .attr('transform', 'translate(' + margin.left + ', '+ ((height - margin.top) + 20) +')');

        legend.append('text')
          .attr('class', 'legend-text')
          .text(chartLegendTitle)
          .attr('y', -5);

        var j = 0;
        for(var m = 0; m < 7; m++) {
          legend.append('rect')
            .attr('transform', 'translate('+ (31 * j) +', 0)')
            .attr('width', 30)
            .attr('height', 8)
            .attr('fill', color[m]);

          legend.append('text')
            .attr('class', 'legend-text')
            .attr('x', (30 * j) + 20)
            .attr('y', 17)
            .text(function() {
              return (m * 5 == 30) ? '30+' : (m * 5);
            });

          j++;
        }
      }

      // Bar chart.
      function barChartInfo(categorize) {
        // Add legend.
        var yCaption = 'Number of Experimental Units';
        var bcLegend;

        if (categorize == 'location' || typeof categorize === 'undefined') {
          bcLegend = dataByTitles;
        }
        else {
         bcLegend = [];
          d3.map(dataByTitles, function(d) {
            bcLegend.push({'key' : d.key});
          });
        }

        var legend = bsvg.append('g')
          .attr('id', 'barchart-legend-rect');

        var legendText = bsvg.append('g')
          .attr('id', 'barchart-legend-text');

        legend.selectAll('rect')
          .data(bcLegend)
			    .enter()
			    .append('rect')
			    .attr('id', function(d, i) {
			      return 'legend-rect-' + i;
			    })
			    .on('mousemove', highlight)
			    .on('mouseout', nohighlight)
			    .attr('fill', function(d, i) { return barchartColor(i); })
			    .attr('height', 14)
			    .attr('width', 15)
			    .attr('x', 0)
			    .attr('y', function(d, i) { return 17 * i; })

	      legendText.selectAll('text')
	        .data(bcLegend)
	        .enter()
	        .append('text')
	        .attr('id', function(d, i) {
			      return 'legend-text-' + i;
			    })
			    .on('mousemove', highlight)
			    .on('mouseout', nohighlight)
	        .text(function(d, i) {
	          // Trim location when it is too long.
	          var loc = d.key;
	          return (loc.length >= 28) ? loc.substr(0, 25) + '...' : loc;
	        })
          .attr('x', 19)
          .attr('class', 'legend-text')
			    .attr('y', function(d, i) { return 17 * i; });

        bsvg.append('g').append('text')
          .attr('class', 'chart-axes')
          .attr('id', 'y-caption')
          .attr('transform', function() {
            return 'translate(45, '+ Math.round(height/2) +') rotate(-90)';
          })
          .text(yCaption);

        // Location - x axis.
        var baseUnit = extractBaseUnit(traitSelectedName);

        bsvg.append('g').append('text')
          .attr('id', 'x-caption')
          .attr('class', 'chart-axes')
          .text('Average Observed Measurements per Experiment Unit ' + baseUnit);
      }



      // Helper functions:
      // Get the unit part given a trait (unit) string.
      // Function highlight rect corresponding to an item in the legend.
      function nohighlight() {
        d3.selectAll('.rect-each-bar')
          .transition()
          .duration(function() { return randomNumber(); })
          .ease('back')
          .style('opacity', 1);
      }

      function highlight() {
        var id = d3.select(this).attr('id');
        var rect_id = '#' + id.replace(/text/i, 'rect');
        var fill = d3.select(rect_id).attr('fill');

        // Lower the opacity of all rect except rect with fill above.
        d3.selectAll('rect.rect-each-bar')
          .transition()
          .duration(function() { return randomNumber(); })
          .ease('back')
          .style('opacity', function() {
            var m = d3.select(this).attr('fill');
            return (m == fill) ? 1 : 0.1;
          });
      }

      function extractBaseUnit(traitName) {
        var regExp = /\(.*\)/;
        var matches = regExp.exec(traitName);
        // The unit might contain other text information.
        // In AGILE project - R1, R3, R5, R7, 1st, 2nd
        var u = matches[0];
        var baseUnit;
        baseUnit = u.replace(/(R1|R3|R5|R7|1st|2nd);\s/i, '');

        return baseUnit;
      }

      // Mark/hightlight reps and add informaton about the marker.
      function markRep(project, trait_id, trait_name) {
        d3.select('#text-not-found').remove();

        // Selected trait name.
		    traitSelectedName = trait_name;

		    // Get selected trait. Trait select box returns cvterm id of a trait.
        traitSelectedId = trait_id;
        traitSelectedId.toString();

        if (traitSelectedId == '0') {
          // First option in the select box.
          // Remove all markers
          delMarker('');
          // Add Information window.
		      infoWindow('off');
        }
        else {
          // Find trait selected in all rep and mark when present.
          var countRep = 0;
          for (var i = 1; i < traitsList.length; i++) {
            var id = '#obj-' + i.toString();

            if (traitsList[i].toString().indexOf(traitSelectedId) !== -1) {
              countRep++;
              // Trait present
              // Add marker if none is present.
              addMarker(id);
            }
            else {
              // Not here.
              // Remove maker if marker is present.
              delMarker(id);
            }
          }
        }

        // Add information window when marker is present.
        if (countRep > 0) {
          // Make the trait selected title of the info window.
          $('#title-pheno').text(function() {
            infoWindow();
            // Tell user how many reps a trait was measured.
            $('#text-rep').text(countRep + ' Rep');

            return traitSelectedName;
          });
        }

       // Test if barchart is part of the DOM already.
       var containerBarchart = ($('#container-barchart').length > 0) ? 1 : 0;
       if (containerBarchart > 0 && traitSelectedId === '0') {
         // Remove barchart when it is present and 'select a trait' option is selected.
         d3.select('#container-barchart')
           .transition()
           .style('opacity', 0)
           .remove();
        }
        else if (traitSelectedId !== '0') {
          // If trait selected is not equals to 'select a trait'.
          if (containerBarchart < 1) {
            // No barchart present in the DOM, Add chart elements.
            d3.select('.container-contents')
              .append('div')
                .attr('id', 'container-barchart')
                .transition()
                .style('opacity',1);

            d3.select('#container-barchart')
              .append('svg')
              .attr('class', 'bar-chart');
          }
          else {
            // Remove previous barchart g container elements.
            // Leave the svg canvas and append a new set of chart elements below.
            d3.selectAll('.bar-chart g, #container-barchart h2').remove();
          }
        }
      }

      // Show or hide the marker information window.
      function infoWindow(state) {
        var window = $('#container-marker-information');

        if (state == 'off') {
          window.slideUp(300);

          // Reset selectbox
          $('select[name=rawdata-sel-trait]').val('');
        }
        else {
          window.slideDown(300);
        }
      }

      // Remove marker.
      function delMarker(id) {
        var m;
        if (id == '') {
          id = '.marker-rep';
          m = d3.selectAll(id);
        }
        else {
          id = id + ' path';
          m = d3.select(id);
        }

        if (m.size() > 0) {
          m
            .transition()
            .duration(function() { return randomNumber(); })
            .style('opacity', 0)
            .remove();
        }
      }

      // Mark rep when trait selected is measured in this rep.
      function addMarker(id) {
        if (d3.select(id + ' path').size() <= 0) {
          var rect = d3.select(id + ' rect');

          if (rect.size() > 0) {
            var x = parseInt(rect.attr('x'));
            var y = parseInt(rect.attr('y'));

            d3.select(id)
              .append('path')
                .attr('class', 'marker-rep')
                .attr('d', leaf)
                .attr('transform', function() {
                  // MARKER DIMENSION: 20X20 pixels
                  // Location of rect plus half the bar width less the 1/2 width of the marker
                  x = x + Math.round(rectDimension.width/2) - 10;
                  y = y + Math.round(rectDimension.height/2) - 15;

                  return 'translate(' + x + ',' + y + ')';
                })
                .style('opacity', 0)
                .transition()
                .duration(function() { return randomNumber(); })
                .style('opacity', 1)
                .attr('filter', 'url(#dropshadow)');
           }
        }
      }

      // Map trait count to colour code.
      // Return number as an index to a color element
      // in color array.
      function setColour(num) {
       if (num == 0)
         return 0;
       else if (num >= 1 && num <= 5)
         return 1;
       else if (num >= 6 && num <= 10)
         return 2;
       else if (num >= 11 && num <= 15)
         return 3;
       else if (num >= 16 && num <= 20)
         return 4;
       else if (num >= 21 && num <= 25)
         return 5;
       else if (num >= 26)
         return 6;
      }

      // Generate random numbers.
      function randomNumber() {
        var min = 1;
        var max = 10;

        return (Math.random() * (max - min) + min ) * 100;
      }

      // Get the stock count of a location in a bin.
      function getStockCount(bin, loc) {
        var d = dataByBins;
        for(var i = 0; i < d.length; i++) {
          if (d[i].key == bin) {
            for(var j = 0; j < d[i].values.length; j++) {
              var m = d3.values(d[i].values[j]);
              if (m[0] == loc) {
                return m[1];
              }
            }

            // Just search in the requested bin.
            return 0;
          }
        }
      }

      // Text and icon shown when there is no data.
      function noData(canvas) {
        // Remove loading animation.
        $('.win-loading').remove();

        var s = (canvas == '.data-chart') ? '.data-chart' : 'svg:last-child';

        d3.select(s)
          .attr('width', width)
          .attr('height', 150);

        var message = d3.select(canvas)
          .append('g')
          .attr('transform', 'translate('+ (Math.floor(width/2) - 40) +', 50)');

        message
          .append('text')
          .attr('class', 'chart-axes')
          .attr('y', 20)
          .attr('x', 50)
          .text('Could not visualize data.');
      }

      // Debugging function. Echo the contents of d.
      function echo(d) {
        alert(JSON.stringify(d));
        //console.log(JSON.stringify(d));
        //console.log(d);
      }
      ////

      // Save the screen/window width to a cookie and read this
      // value to let php decide bin size information.
      function storeScreenWidth(winWidth) {
        document.cookie = 'rawphenoRawdataSW=' + winWidth + ';path=/;domain=.knowpulse.usask.ca';
      }

      // Populate select box based on categorized value.
      // Location + Year by default.
      function barchartFillSelect(category, dataCategory) {
        // Reference select option.
        var selOption = $('#sel-barchart-option');

        if (category == 'location') {
          // Fill the select box with years.
          $(dataCategory.planting_date).each(function(i, v) {
            selOption.append($('<option>').attr('value', v).text(v));
          });
        }
        else if(category == 'year') {
          // Fill the select box with location.
          $(dataCategory.location).each(function(i, v) {
            selOption.append($('<option>').attr('value', v).text(v));
          });
        }
      }

      // Categorize chart.
      function barchartCategorize(categoryValue, optionValue, sel, dataCategory, pId, tId){
        // Category option has been changed.
        // When category select is selected, reload the barchart and default
        // to location and first option in the year select box.
        if (sel == 'sel-barchart-category') {
          // Reset the options in select options.
          $('#sel-barchart-option').find('option').remove();

          // Category select selected.
          // Re populate the select option select box.
          barchartFillSelect(categoryValue, dataCategory);

          // Default to corresponding values.
          optionValue = (categoryValue == 'location')
            ? dataCategory.planting_date[0]
            : dataCategory.location[0];
        }

        // Read the JSON given the values from select boxes.
        var barchartFile = file + 'rawdata_trait' + '?project_id=' + pId + '&trait_id=' + tId + '&category=' + categoryValue + '&option=' + optionValue;

        // Render the barchart.
        d3.json(barchartFile, function(error, barchartData) {
          if (error) {
            // Error reading JSON.
            throw error;
          }
          else if(barchartData == 0) {
            // No data.
            noData('.bar-chart');
          }
          else {
            // Reset the stage.
            $('#container-barchart').find('g, h2, .messages').remove();

            // Show please wait animation...
            loading('barchart');

            // Initialize barchart by adding all elements (rect, g, text, etc.) into the DOM,
            // and then call render function and position these elements in the right place.
            initializeBarChart(barchartData, dataCategory, pId, tId);
          }
        });
      }

      // Show please wait animation while chart is processing.
      function loading(chart) {
         var container = (chart == 'heatmap')
           ? '#container-rawdata'
           : '#container-barchart';

         d3.select(container)
           .append('div')
           .attr('class', 'win-loading')
           .html('Please wait...');
      }

      // Wrap long text value and set the first line
      // to bold and capitalized.
      function wrapWords(text) {
        text.each(function() {
          // Reference text.
          var text  = d3.select(this);
          // Read the words in the text.
          var words = text.text().split(',');

          // Clear the text so no duplicate label shown.
          text.text(null);

          var word,
            line = [],
            lineNumber = 0,
            lineHeight = 1.2 // ems
            y = text.attr('y') - 10,
            dy = parseFloat(text.attr('dy'));

          while (word = words.pop()) {
            text.append('tspan')
              .attr('class', 'bp-tspan')
              .attr('x', 0)
              .attr('y', y)
              .attr('dy', ++lineNumber * lineHeight + dy + 'em')
              .text(word.trim())
                .attr('class', function() {
                  return (lineNumber - 1 == 0) ? 'location-text-top' : 'location-text-bottom';
                });
          }
         });
       }
    }
  };
}(jQuery));
