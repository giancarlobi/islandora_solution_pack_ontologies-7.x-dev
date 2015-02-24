Version beta 0.2

A simplified version of great work of Diego.

Extract Class from Ontology and create Class object.

Association between Class object and Model.

TODO

Modify relationships edit form


RESTRICTION in OWL Ontology
---------------------------

SUBJECT : PROPERTY : OBJECT

- Property must be an "Object Property"

- Subject and Object must have model associated to a class

- owl:Restriction (class based) the only one applied

- if Property P NOT present in a <owl:Restriction> tag then  ANY : P : ANY 

- if <owl:Class A>
       <rdfs:subClassOf>
         <owl:Restriction>
           <owl:onProperty P />
           <owl:allValuesFrom B />
         </owl:Restriction>
       </rdfs:subClassOf>
     </owl:Class>

  then
 		A : P : B
    or   		
		ANY != A : P : ANY










