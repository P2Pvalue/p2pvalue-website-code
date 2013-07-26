# VisualScience

A module providing an easy-to-use intuitive search interface for your Drupal Database. With just a single search box and advanced autocomplete you can get the targeted list of entities (users or nodes) within several seconds. You can also save your searches to have quick access to commonly used lists. **No configuration needed!**

Features include:
+	Search
+	Smart Autocomplete (query building tool)
+	Save/Load Searches
+	Multiple Visualisation Mechamisms & Extendability (see **UserList** module)
+	Multiple Searches on the same page (Window-based system)

## Search

Use Refine Search for filtering the list according to the given criteria. The search syntax is the following:

**FIELD_NAME_1**=[**TERM_11** AND **TERM_12** OR **TERM_13**...] OR **FIELD_NAME_2**=[**TERM_21**] AND ...

**FIELD_NAME** corresponds either to the name of the field in the database, declared as field_**FIELD_NAME**, or one of the following fields of the **users** table: **name**, **email**, **role**, **date** (of registration). For the date a special syntax is used: date=[START_DATE, END_DATE]. If the START_DATE is skipped, i.e. date=[,END_DATE], there won't be a START_DATE used, and if END_DATE is skipped, i.e. date=[START_DATE,], as the upper limit the current date will be used. If only one date is given, i.e. date=[DATE], then it will be used as the starting date. All the PHP supported syntaxes can be used to specify a date. A calendar is provided while typing.

The same FIELD_NAME can be used several times. For **AND** and **OR** also "&&", "&" and "||", "|" can be used accordingly. The search and the logical operators are **not case sensitive**. 

**Example:** country=[Switzerland || Armenia] && role=[scientist] OR institution=[ETH Zurich]. 
This will result in a list of scientist from Switzerland and Armenia + everybody from ETH Zurich  

**NOT** logical operation can be also used with the terms, in the following form: **NOT(TERM)** 

## Autocomplete

While typing in the search box, smart autocomplete is provided. 
+	After typing FIELD_NAME=, "[" is added automatically. 
+	The list is provided according to your database entries. 
+	**Multiple options** can be selected by checking (SPACEBAR must be used) the **checkboxes** in front of the terms. 
+	To confirm the selection press ENTER. The multiple selected terms are added with **AND** logical operator. 
+	For the "date" a calendar is provided.


## Save/Load
In the Save field you can save the current search query or load a previously saved search. 
+	**To save**: Type the name of the search in the given text field and press Save button to save the current search in the database. 
+	**To load**: Just select the name of the previously saved search from the given drop-down list.


## UserList Module
This is the main visualization module for visualscience. The search results are displayed via the UserList module. It is easily extendible to add different actions and visualisation mechanisms to the list of results. For more information see the UserList module itself
