custom-fedex
============

A custom shipping module for fedex

The Aim of this project is to have a shipping module that works internally and in conjuction with fedex shipping

Workflow

1. When an order is made the product' SKU is taken and is then run through a our internal packaging system that groups products together based on sizes and other parameters and packages

2. The final size of the package is then estimated in terms of weight and size and the details are sent to fedex webservices alongside with the shipping address

3. We then receive a reply as to how much it will cost to ship the final package to the said address and the quote is added to the shipping in drupal commerce


Available script
We currently have the SOAP 1.1 and 1.2 script with the wsdl file that performs stage 2 of the workflow

What we need to do is to let the module pick the SKU and the address of the ordered product and send it to the WSDL as "Part_ID". 
The WSDL then does it job and returns the shipping quote back to the module and the module adds the quote as the shipping cost


