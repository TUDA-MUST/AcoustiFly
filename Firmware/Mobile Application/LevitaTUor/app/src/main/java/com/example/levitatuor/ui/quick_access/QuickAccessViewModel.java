package com.example.levitatuor.ui.quick_access;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

public class QuickAccessViewModel extends ViewModel {

    private final MutableLiveData<String> mText;

    public QuickAccessViewModel() {
        mText = new MutableLiveData<>();
        mText.setValue("This is quick_access fragment");
    }

    public LiveData<String> getText() {
        return mText;
    }
}