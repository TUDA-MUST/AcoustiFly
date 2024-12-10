package com.example.levitatuor.ui.quick_access;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import com.example.levitatuor.databinding.FragmentQuickAccessBinding;

public class QuickAccessFragment extends Fragment {

    private FragmentQuickAccessBinding binding;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        QuickAccessViewModel quickAccessViewModel =
                new ViewModelProvider(this).get(QuickAccessViewModel.class);

        binding = FragmentQuickAccessBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        final TextView textView = binding.textSlideshow;
        quickAccessViewModel.getText().observe(getViewLifecycleOwner(), textView::setText);
        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        //binding = null;
    }
}