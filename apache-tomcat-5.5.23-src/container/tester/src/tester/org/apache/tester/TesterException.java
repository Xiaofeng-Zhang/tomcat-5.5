/*
 * Copyright 1999, 2000, 2001 ,2004 The Apache Software Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.tester;


import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;

/**
 * Generic exception class to use for testing error page assertions.
 *
 * @author Craig R. McClanahan
 * @version $Revision: 302726 $ $Date: 2004-02-27 07:59:07 -0700 (Fri, 27 Feb 2004) $
 */

public class TesterException extends Exception {


    /**
     * Construct an exception with no associated message.
     */
    public TesterException() {

        super();

    }


    /**
     * Construct an exception with the associated message.
     *
     * @param message The associated message
     */
    public TesterException(String message) {

        super(message);

    }


}