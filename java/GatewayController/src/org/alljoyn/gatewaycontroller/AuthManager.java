/******************************************************************************
 * Copyright (c) Open Connectivity Foundation (OCF) and AllJoyn Open
 *    Source Project (AJOSP) Contributors and others.
 *
 *    SPDX-License-Identifier: Apache-2.0
 *
 *    All rights reserved. This program and the accompanying materials are
 *    made available under the terms of the Apache License, Version 2.0
 *    which accompanies this distribution, and is available at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Copyright (c) Open Connectivity Foundation and Contributors to AllSeen
 *    Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for
 *    any purpose with or without fee is hereby granted, provided that the
 *    above copyright notice and this permission notice appear in all
 *    copies.
 *
 *     THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 *     WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 *     WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 *     AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 *     DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 *     PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 *     TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 *     PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

package org.alljoyn.gatewaycontroller;

import java.util.Arrays;
import java.util.List;

import org.alljoyn.bus.AuthListener;
import org.alljoyn.bus.BusAttachment;
import org.alljoyn.bus.BusException;
import org.alljoyn.bus.Status;
import org.alljoyn.config.ConfigServiceImpl;
import org.alljoyn.config.client.ConfigClient;
import org.alljoyn.config.client.ConfigClientImpl;
import org.alljoyn.gatewaycontroller.sdk.GatewayController;
import org.alljoyn.gatewaycontroller.sdk.GatewayMgmtApp;
import org.alljoyn.services.android.security.AuthPasswordHandler;
import org.alljoyn.services.android.security.SrpAnonymousKeyListener;
import org.alljoyn.services.android.utils.AndroidLogger;
import org.alljoyn.services.common.ServiceAvailabilityListener;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.util.Log;

/**
 * Registers {@link AuthListener}. The default passcode is defined as in
 * {@link SrpAnonymousKeyListener#DEFAULT_PINCODE} If
 * {@link AuthListener#completed(String, String, boolean)} method call is
 * received with authenticated flag of FALSE, then the
 * {@link GWControllerActions#GWC_PASSWORD_REQUIRED} intent is broadcasted.
 */
public class AuthManager implements AuthPasswordHandler {
    private static final String TAG = "gwcapp" + AuthManager.class.getSimpleName();

    /**
     * The context object which is used to broadcast {@link Intent}
     */
    private Context context;

    /**
     * Current pass code of a Gateway Agent
     */
    private char[] passCode;

    /**
     * The authentication mechanisms that this application supports
     */
    private static final String[] AUTH_MECHANISMS = new String[] { "ALLJOYN_SRP_KEYX", "ALLJOYN_ECDHE_PSK" };

    private SharedPreferences sharedPrefs;

    /**
     * Constructor
     * 
     * @param context
     *            The {@link Context} object to be used for {@link Intent}
     *            broadcasting
     */
    public AuthManager(Context context) {

        this.context = context;
        this.passCode = SrpAnonymousKeyListener.DEFAULT_PINCODE;
    }

    private ConfigClient configClient;

    public void connectConfigSession(String busName) throws Exception {
        if (busName == null) {
            throw new IllegalArgumentException("busattachment name is null");
        }

        if (configClient != null && configClient.getPeerName().equals(busName)) {
            if (configClient.isConnected()) {
                return;
            }
        } else {
            try {
                disconnectConfigSession();
                configClient = ConfigServiceImpl.getInstance().createFeatureConfigClient(busName, null,
                        GatewayController.PORT_NUM);
                configClient.connect();
            } catch (Exception e) {
                Log.e(TAG, "could not create feature config client for bus " + busName);
                Log.e(TAG, e.toString());
            }
        }

    }

    public void disconnectConfigSession() {
        if (configClient != null) {
            configClient.disconnect();
        }

        configClient = null;
    }

    public static final String INTENT_EXTRA_APP_ID = "extraAppId";

    /**
     * Set current pass code to authenticate a Gateway Agent
     * 
     * @param passCode
     * @param busName
     * @throws IllegalArgumentException
     *             if the received passCode is undefined or if busname is null
     */
    public void setPassCode(String passCode, String busName, String appId) {
        if (passCode == null) {
            throw new IllegalArgumentException("passcode is null");
        }

        //this.passCode = passCode.toCharArray();

        SharedPreferences.Editor editor = context.getSharedPreferences(SHARED_PREFERENCES_PASSCODES_NAME, Context.MODE_PRIVATE).edit();
        try {

            connectConfigSession(busName);

            if (!configClient.isConnected()) {
                Log.i(TAG, "Config Client is Not Connected");
                return;
            }

            configClient.setPasscode("", passCode.toCharArray());

            //TODO SAVE PASSCODE
            
            editor.putString(appId, passCode);
            if (!editor.commit()) {
                throw new Exception("passcode not saved");
            }

        } catch (BusException e) {
            Log.e(TAG, "Setting passcode failed");
            editor.remove(appId);
            editor.apply();
            Intent intent = new Intent(GWControllerActions.GWC_SET_PASSCODE_FAILED.name());
            intent.putExtra(INTENT_EXTRA_APP_ID, appId);
            context.sendBroadcast(intent);
            Log.e(TAG, e.toString());
        } catch (Exception e) {
            editor.remove(appId);
            editor.apply();
            Intent intent = new Intent(GWControllerActions.GWC_SET_PASSCODE_FAILED.name());
            intent.putExtra(INTENT_EXTRA_APP_ID, appId);
            context.sendBroadcast(intent);
            Log.e(TAG, "Setting passcode failed");
        }

    }

    /**
     * Resets current passcode to default passcode
     * 
     * @param busName
     * @throws IllegalArgumentException
     *             if busName is null
     */
    public void doFactoryReset(String busName) {

        passCode = SrpAnonymousKeyListener.DEFAULT_PINCODE;
        try {
            connectConfigSession(busName);

            if (!configClient.isConnected()) {
                Log.i(TAG, "Config Client is Not Connected");
                return;
            }

            configClient.factoryReset();
        } catch (BusException e) {
            // TODO Auto-generated catch block
            Log.e(TAG, e.toString());
            e.printStackTrace();
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

    }

    /**
     * Register the AuthManager
     * 
     * @param bus
     *            {@link BusAttachment} to be used for the registration
     * @return {@link Status}
     */
    public Status register(BusAttachment bus) {

        try {
            ConfigServiceImpl.getInstance().startConfigClient(bus);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        SrpAnonymousKeyListener authListener = new SrpAnonymousKeyListener(this, new AndroidLogger(), AUTH_MECHANISMS);

        String keyStoreFileName = context.getFileStreamPath("alljoyn_keystore").getAbsolutePath();
        Status status = bus.registerAuthListener(authListener.getAuthMechanismsAsString(), authListener,
                keyStoreFileName);

        Log.d(TAG, "AuthListener has registered, Status: '" + status + "'");
        return status;
    }
    
    public static final String SHARED_PREFERENCES_PASSCODES_NAME = "GWCSharedPreferencesPasscodes";

    /**
     * @see org.alljoyn.services.android.security.AuthPasswordHandler#getPassword(java.lang.String)
     */
    @Override
    public char[] getPassword(String busName) {
        
        List<GatewayMgmtApp> gatewayApps = GatewayController.getInstance().getGatewayMgmtApps();
        
        String appId = "";
        for (GatewayMgmtApp gw: gatewayApps) {
            if (gw.getBusName().equals(busName)) {
                appId = gw.getAppId().toString();
                break;
            }
        }
        
        String passCode = context.getSharedPreferences(SHARED_PREFERENCES_PASSCODES_NAME, Context.MODE_PRIVATE).getString(appId, "");
        Log.i(TAG, "passcode is " + passCode);

        if (passCode.equals(""))
            return SrpAnonymousKeyListener.DEFAULT_PINCODE;
        return passCode.toCharArray();
    }

    /**
     * @see org.alljoyn.services.android.security.AuthPasswordHandler#completed(java.lang.String,
     *      java.lang.String, boolean)
     */
    @Override
    public void completed(String mechanism, String peerName, boolean authenticated) {

        if (authenticated) {
            Log.d(TAG, "The authentication process has been completed successfully. Mechanism: '" + mechanism
                    + "' ,peerName: '" + peerName + "'");
            return;
        }

        Log.d(TAG, "The authentication process has FAILED . Mechanism: '" + mechanism + "' ,peerName: '" + peerName
                + "' broadcasting Intent");

        Intent intent = new Intent(GWControllerActions.GWC_PASSWORD_REQUIRED.name());
        context.sendBroadcast(intent);
    }

}