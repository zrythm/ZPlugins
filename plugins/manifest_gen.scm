#!@GUILE@ -s
!#
;;;
;;; Copyright (C) 2020 Alexandros Theodotou <alex at zrythm dot org>
;;;
;;; This file is part of ZPlugins.
;;;
;;; ZPlugins is free software; you can redistribute it and/or modify it
;;; under the terms of the GNU Affero General Public License as published by
;;; the Free Software Foundation; either version 3 of the License, or (at
;;; your option) any later version.
;;;
;;; ZPlugins is distributed in the hope that it will be useful, but
;;; WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU Affero General Public License for more details.
;;;
;;; You should have received a copy of the GNU Affero General Public License
;;; along with ZPlugins.  If not, see <http://www.gnu.org/licenses/>.

;;; This file returns the manifest.ttl for a plugin

(define-module (zplugins mainfest-gen)
  #:use-module (ice-9 format)
  #:use-module (ice-9 match))

(define (arg-exists arg1)
  (not (string=? arg1 "none")))

#!
Args:
1: output file
2: project URI
3: plugin type
4: plugin URI
5: dsp binary
6: plugin minor version
7: plugin micro version
8: plugin ttl
9: the UI type or "none"
10: UI URI or "none"
11: ui binary or "none"
12: presets file or "none"
!#
(define (main . args)

  ;; verify number of args
  (when (not (eq? (length args) 13))
    ((display "Need 12 arguments")
     (newline)
     (exit -1)))

  ;; get args
  (match args
    ((this-program output-file project-uri
                   plugin-type plugin-uri
                   dsp-binary
                   minor-ver micro-ver plugin-ttl
                   ui-type ui-uri ui-binary
                   presets-file)

     ;; open file
     (with-output-to-file output-file
       (lambda ()

         ;; print includes
         (format #t
"@prefix doap: <http://usefulinc.com/ns/doap#> .
@prefix foaf: <http://xmlns.com/foaf/0.1/> .
@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .
@prefix pset: <http://lv2plug.in/ns/ext/presets#> .
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .
@prefix ui:   <http://lv2plug.in/ns/extensions/ui#> .

<~a>
  a doap:Project ;
  doap:name \"Zrythm plugins\" ;
  doap:description \"\"\"
Collection of LV2 plugins to be bundled with Zrythm
\"\"\" ;
  doap:maintainer [
    foaf:name \"\"\"Alexandros Theodotou\"\"\" ;
    foaf:homepage <https://www.zrythm.org> ;
  ] ;
  doap:repository [
    doap:browse <https://git.zrythm.org/cgit/zplugins> ;
    doap:location <https://git.zrythm.org/git/zplugins> ;
  ] .

" project-uri)

         ;; print main URI
         (format #t "<~a>
  a lv2:Plugin,
    lv2:~a ;
  lv2:binary <~a> ;
" plugin-uri plugin-type dsp-binary)
         (when (arg-exists ui-uri)
           (format #t "  ui:ui <~a> ;\n"
                   ui-uri))
         (format #t "  lv2:minorVersion ~a;
  lv2:microVersion ~a;
  rdfs:seeAlso <~a> .

" minor-ver micro-ver plugin-ttl)

         ;; print UI URI
         (when (arg-exists ui-uri)
           (format #t "<~a>
  a ui:~a ;
  ui:binary <~a> ;
  lv2:requiredFeature ui:idleInterface ;
  lv2:extensionData ui:resize ;
  rdfs:seeAlso <~a> .
" ui-uri ui-type ui-binary plugin-ttl))

         ;; add presets
         (when (file-exists? presets-file)
           (begin
             (newline)
             (with-input-from-file presets-file
               (lambda ()
                 (let loop ((x (read-char)))
                   (when (not (eof-object? x))
                     (begin
                       (display x)
                       (loop (read-char))))))))))))))


(apply main (program-arguments))
