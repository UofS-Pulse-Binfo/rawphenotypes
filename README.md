# Raw Phenotypes
This module was designed for storing the raw phenotpic data being collected as part of ["Application of genomics to innovation in the Lentil Economy (AGILE)" project](http://knowpulse.usask.ca/portal/project/AGILE%3A-Application-of-Genomic-Innovation-in-the-Lentil-Economy). 

__This module is currently not ready for use outside of KnowPulse. If you are interested in our functionality, feel free to contact us.__

## Dependencies
1. [Drag & Drop Upload](https://www.drupal.org/project/dragndrop_upload)
2. [Spreadsheet Reader](https://github.com/nuovo/spreadsheet-reader)

__Note: there are some modifications needed to the spreadsheet reader to get it to handle dates the way we expect. There will be a patch available in the future.__

## Features
- A d3.js heatmap summarizes the raw data available by displaying the number of traits broken down by location (x-axis) and replicate (y-axis; grouped by year). This chart uses a materialized view for improved performance.
- Upload data functionality supporting excel spreadsheets (XLSX). Currently the loader expects the traits required for the AGILE project but it also flexible enough to allow users to add additional traits (one per column) to the spreadsheet. If additional traits are present, the loader asks the user to describe the trait including the units and any scale used.
- During upload the file is validated using a number of tests including general "Is this an excel file?", as well as, "Do all the germplasm in the file already exist in the chado.stock table?". Validation tests are provided via a Drupal hook meaning you can add your own data-specific tests (see github wiki).
- A searchable trait collection instruction page defining a standard protocol for collecting of the traits required by AGILE. In the future, the intent is for these instructions to be easily customizable -perhaps even drawn from the trait descriptions in chado.
- Data Download functionality which allows users to select the locations and traits they are interested in. A comma-separated file is produced which can easily be opened in excel for viewing and is R-friendly for analysis.
